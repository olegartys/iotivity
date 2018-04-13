#include <string>
#include <fstream>
#include <sstream>
#include <mutex>
#include <memory>
#include <condition_variable>

#include <HAL/shouse_res_hal_dll.h>

#include <shouse_res_client.h>
#include <shouse_res_srv.h>
#include <shouse_default_platform.h>

using namespace OC;

static constexpr const char* LOG_TAG = "ShouseDefaultPlatform";


static std::string readFile(const std::string& configPath) {
    std::ifstream config(configPath);
    std::stringstream ss;
    ss << config.rdbuf();
    return ss.str();
}

/******************************************************************************
 * Server resource loader *
 *****************************************************************************/

template<>
std::shared_ptr<ShouseResourceServer>
ShouseDefaultPlatform::LoadResource<PlatformType::SHOUSE_SERVER,
    ShouseResourceServer>(const std::string& configPath) {
    const std::string jsonStr = readFile(configPath);

    ResourceInfoServer resourceInfo;
    if (!resourceInfo.fromJson(jsonStr)) {
        Log::error(LOG_TAG, "Error building resource {} from JSON",
            configPath);
        return nullptr;
    }

    ShouseServerHALdll* dllHAL = new ShouseServerHALdll;
    auto HAL_path = resourceInfo.mHalPath;
    if (!dllHAL->init(HAL_path.c_str())) {
        Log::error(LOG_TAG, "Error loading DLL hal from {}", HAL_path);
        return nullptr;
    }

    auto resourceServer = std::make_shared<ShouseResourceServer>(
        resourceInfo.mName, resourceInfo.mUri, resourceInfo.mType,
        resourceInfo.mIface);
    if (!resourceServer->createResource(dllHAL)) {
        Log::error(LOG_TAG, "Error creating resource from DLL");
        return nullptr;
    }

    return resourceServer;
}

/******************************************************************************
 * Client resource loader *
 *****************************************************************************/

class ResourceHolder {
public:
    void addResource(const std::string& uri,
        std::shared_ptr<OCResource> ocResource) {
        std::lock_guard<std::mutex> lock(mLock);

        // Check whether ResourceInfo structure exists for the given resource

        auto resourceInfoIt = mResourcesInfo.find(uri);
        if (resourceInfoIt == mResourcesInfo.end()) {
            Log::warn(LOG_TAG, "Resource {} found, but no ResourceInfo"
                "was provided", uri);
            return;
        }

        auto resourceInfo = resourceInfoIt->second;

        // Add new resource as a shared pointer. We will always hold one,
        // so the objects will never be destructed and lost

        auto resource = std::make_shared<ShouseResourceClient>(
            resourceInfo.mName, resourceInfo.mUri, resourceInfo.mType,
            resourceInfo.mIface, resourceInfo.mAsync);

        resource->setOCResource(ocResource);

        mResources[uri] = std::move(resource);

        // Notify getResource that new resource with uri = mRecentUriAdded
        // has been added

        Log::info(LOG_TAG, "Resource uri = {} added", uri);

        mRecentUriAdded = uri;
        mResourceWaitCv.notify_all();
    }

    std::shared_ptr<ShouseResourceClient> getResourceSync(
        const ResourceInfoClient& info) {
        auto uri = info.mUri;
        
        // Check whether resource with given uri already stored

        {
            std::lock_guard<std::mutex> lock(mLock);
            mResourcesInfo[uri] = info;
            if (mResources.find(uri) != mResources.end()) {
                // Alright, resource found -> just return pointer

                return mResources[uri];
            }
        }
        
        Log::error(LOG_TAG, "Resource {} was not found", uri);

        // Otherwise, lets wait for the needed resource to be added

        std::unique_lock<std::mutex> lock(mLock);
        mResourceWaitCv.wait(lock, [this, &uri] {
            return mRecentUriAdded == uri;
        });

        return mResources[uri]; 
    }

    const std::map<std::string, std::shared_ptr<ShouseResourceClient>>& getResources() {
        std::lock_guard<std::mutex> lock(mLock);
        return mResources;
    }

private:
    static constexpr const char* LOG_TAG = "ResourceHolder";

    std::map<std::string, ResourceInfoClient> mResourcesInfo;
    std::map<std::string, std::shared_ptr<ShouseResourceClient>> mResources;
    std::mutex mLock;

    std::condition_variable mResourceWaitCv;

    std::string mRecentUriAdded;

};

static ResourceHolder gResourceHolder;


static void onFoundResource(std::shared_ptr<OCResource> resource) {
    Log::debug(LOG_TAG, "{}: resource found {}", __FUNCTION__, resource->uri());

    gResourceHolder.addResource(resource->uri(), resource);
}

static void onFoundResourceError(const std::string& err, const int err_t) {
    Log::error(LOG_TAG, "{}: error {}, {}", __FUNCTION__, err, err_t);
}

template<>
void ShouseDefaultPlatform::RefreshResourcesList<PlatformType::SHOUSE_CLIENT>() {
    std::stringstream requestURI;
    requestURI << OC_RSRVD_WELL_KNOWN_URI; //"/a/light";
    OCPlatform::findResource("", requestURI.str(), CT_DEFAULT, onFoundResource,
                             onFoundResourceError);
}

static bool gObserve;
static void startResourceObservation() {
    Log::info(LOG_TAG, "Starting resources observation...");
    gObserve = true;
    std::thread t([] {
        while (gObserve) {
            ShouseDefaultPlatform::RefreshResourcesList<PlatformType::SHOUSE_CLIENT>();
            sleep(1);
        }
    });
    t.detach();
}

static void stopResourceObservation() {
    Log::info(LOG_TAG, "Stop resource observation");
    gObserve = false;
}

template<>
std::shared_ptr<ShouseResourceClient>
ShouseDefaultPlatform::LoadResource<PlatformType::SHOUSE_CLIENT,
    ShouseResourceClient>(const std::string& configPath) {
    const std::string jsonStr = readFile(configPath);

    ResourceInfoClient resourceInfo;
    if (!resourceInfo.fromJson(jsonStr)) {
        Log::error(LOG_TAG, "Error building resource {} from JSON",
            configPath);
        return nullptr;
    }

    startResourceObservation();

    auto resourceClient = gResourceHolder.getResourceSync(resourceInfo);
    if (!resourceClient) {
        Log::error(LOG_TAG, "Error getting resource {}",
            resourceInfo.mName);
        stopResourceObservation();
        return nullptr;
    }

    stopResourceObservation();

    return resourceClient;
}

/******************************************************************************
 * Client platform configuration *
 *****************************************************************************/

template <> 
int ShouseDefaultPlatform::Configure<PlatformType::SHOUSE_CLIENT>() {
	int ret = 0;

	static OCPersistentStorage ps { client_open, fread, fwrite, fclose, unlink };

    // Create PlatformConfig object
    static PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Both,
        "0.0.0.0",
        0,
        OC::QualityOfService::HighQos,
        &ps
    };

    OCPlatform::Configure(cfg);

    ShouseDefaultPlatform::RefreshResourcesList<PlatformType::SHOUSE_CLIENT>();

	return ret;
}

/******************************************************************************
 * Server platform configuration *
 *****************************************************************************/

template <> 
int ShouseDefaultPlatform::Configure<PlatformType::SHOUSE_SERVER>() {
	int ret = 0;

	// Set of strings for each of platform Info fields
	std::string gPlatformId = "0A3E0D6F-DBF5-404E-8719-D6880042463A";
	std::string gManufacturerName = "OCF";
	std::string gManufacturerLink = "https://www.iotivity.org";
	std::string gModelNumber = "myModelNumber";
	std::string gDateOfManufacture = "2016-01-15";
	std::string gPlatformVersion = "myPlatformVersion";
	std::string gOperatingSystemVersion = "myOS";
	std::string gHardwareVersion = "myHardwareVersion";
	std::string gFirmwareVersion = "1.0";
	std::string gSupportLink = "https://www.iotivity.org";
	std::string gSystemTime = "2016-01-15T11.01";

	// Set of strings for each of device info fields
	std::string  deviceName = "IoTivity Simple Server";
	std::string  deviceType = "oic.wk.tv";
	std::string  specVersion = "ocf.1.1.0";
	std::vector<std::string> dataModelVersions = {"ocf.res.1.1.0", "ocf.sh.1.1.0"};
	std::string  protocolIndependentID = "fa008167-3bbf-4c9d-8604-c9bcb96cb712";

	// Configure platform
    static OCPersistentStorage ps { client_open, fread, fwrite, fclose, unlink };

	// OCPlatformInfo Contains all the platform info to be stored
	static OCPlatformInfo platformInfo;

    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Server,
        "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
        0,         // Uses randomly available port
        OC::QualityOfService::LowQos,
        &ps
    };

    cfg.transportType = static_cast<OCTransportAdapter>(OCTransportAdapter::OC_ADAPTER_IP | OCTransportAdapter::OC_ADAPTER_TCP);

    cfg.QoS = OC::QualityOfService::LowQos;

    OCPlatform::Configure(cfg);

    OCStackResult result = SetPlatformInfo(platformInfo, gPlatformId, gManufacturerName, gManufacturerLink,
        gModelNumber, gDateOfManufacture, gPlatformVersion, gOperatingSystemVersion,
        gHardwareVersion, gFirmwareVersion, gSupportLink, gSystemTime);

    try {
        result = OCPlatform::registerPlatformInfo(platformInfo);
    } catch (OC::OCException& e) {
        Log::error(LOG_TAG, e.what());
        if (result != OC_STACK_OK) {
            Log::error(LOG_TAG, "Platform registration failed");
            return -1;
        }    
    }

	return ret;
}

OCStackResult ShouseDefaultPlatform::SetPlatformInfo(OCPlatformInfo& platformInfo, std::string platformID, std::string manufacturerName,
        std::string manufacturerUrl, std::string modelNumber, std::string dateOfManufacture,
        std::string platformVersion, std::string operatingSystemVersion,
        std::string hardwareVersion, std::string firmwareVersion, std::string supportUrl,
        std::string systemTime) {
	DuplicateString(&platformInfo.platformID, platformID);
	DuplicateString(&platformInfo.manufacturerName, manufacturerName);
	DuplicateString(&platformInfo.manufacturerUrl, manufacturerUrl);
	DuplicateString(&platformInfo.modelNumber, modelNumber);
	DuplicateString(&platformInfo.dateOfManufacture, dateOfManufacture);
	DuplicateString(&platformInfo.platformVersion, platformVersion);
	DuplicateString(&platformInfo.operatingSystemVersion, operatingSystemVersion);
	DuplicateString(&platformInfo.hardwareVersion, hardwareVersion);
	DuplicateString(&platformInfo.firmwareVersion, firmwareVersion);
	DuplicateString(&platformInfo.supportUrl, supportUrl);
	DuplicateString(&platformInfo.systemTime, systemTime);

	return OC_STACK_OK;
}

void ShouseDefaultPlatform::DuplicateString(char ** targetString, std::string sourceString) {
    *targetString = new char[sourceString.length() + 1];
    strncpy(*targetString, sourceString.c_str(), (sourceString.length() + 1));
}
