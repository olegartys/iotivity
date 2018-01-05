//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

///
/// This sample provides steps to define an interface for a resource
/// (properties and methods) and host this resource on the server.
///
#include "iotivity_config.h"

#include <functional>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#include <mutex>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include "baseresource.h"
#include "shouse_res_hal.h"
#include "shouse_res_srv.h"

#include "ocpayload.h"

using namespace OC;
using namespace std;
namespace PH = std::placeholders;

static const char* SVR_DB_FILE_NAME = "./oic_svr_db_server.dat";
static const char* LOG_TAG = "test_server";

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

// OCPlatformInfo Contains all the platform info to be stored
OCPlatformInfo platformInfo;

// OCDeviceInfo Contains all the device info to be stored
OCDeviceInfo deviceInfo;

static FILE* client_open(const char* /*path*/, const char *mode)
{
    return fopen(SVR_DB_FILE_NAME, mode);
}

void DuplicateString(char ** targetString, std::string sourceString)
{
    *targetString = new char[sourceString.length() + 1];
    strncpy(*targetString, sourceString.c_str(), (sourceString.length() + 1));
}

OCStackResult SetPlatformInfo(std::string platformID, std::string manufacturerName,
        std::string manufacturerUrl, std::string modelNumber, std::string dateOfManufacture,
        std::string platformVersion, std::string operatingSystemVersion,
        std::string hardwareVersion, std::string firmwareVersion, std::string supportUrl,
        std::string systemTime)
{
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

class LightHAL : public ShouseResHAL {
public:
    int onGet(OCRepresentation& curRepr, const QueryParamsMap& params) noexcept override {
        int ret = 0;

        Log::info(LOG_TAG, "Get something");

        return ret;
    }

    int onPut(OCRepresentation& curRepr, const QueryParamsMap& params) noexcept override {
        int ret = 0;

        Log::info(LOG_TAG, "Put something");

        return ret;        
    }

    int serialize(OCRepresentation& curRepr) override {
        int ret = 0;

        return ret;
    }
};

int main(int argc, char** argv) {
    // Configure platform
    OCPersistentStorage ps {client_open, fread, fwrite, fclose, unlink };

    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Server,
        "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
        0,         // Uses randomly available port
        OC::QualityOfService::LowQos,
        &ps
    };

    cfg.transportType = static_cast<OCTransportAdapter>(OCTransportAdapter::OC_ADAPTER_IP | 
                                                    OCTransportAdapter::OC_ADAPTER_TCP);

    cfg.QoS = OC::QualityOfService::LowQos;

    OCPlatform::Configure(cfg);

    OCStackResult result = SetPlatformInfo(gPlatformId, gManufacturerName, gManufacturerLink,
        gModelNumber, gDateOfManufacture, gPlatformVersion, gOperatingSystemVersion,
        gHardwareVersion, gFirmwareVersion, gSupportLink, gSystemTime);

    try {
        result = OCPlatform::registerPlatformInfo(platformInfo);
    } catch (OC::OCException& e) {
        Log::error("Execption: %s", e.what());
        if (result != OC_STACK_OK) {
            Log::error(LOG_TAG, "Platform registration failed");
            return -1;
        }    
    }


    
    // Create resource
    BinaryLightServer light("/a/light");
    light.createResource();
    light.disable();

    // Start listen
    Log::info(LOG_TAG, "Listening...");
    while(1);

    return 0;
}
