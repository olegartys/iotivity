#ifndef SHOUSE_PLATFORM_H
#define SHOUSE_PLATFORM_H

#include <string>
#include <cstdio>

#include <OCApi.h>
#include <OCPlatform.h>

#include <cJSON.h>

#include <shouse/Log.h>

enum class PlatformType {
	SHOUSE_CLIENT, SHOUSE_SERVER
};

class ShouseDefaultPlatform final {
public:
	ShouseDefaultPlatform() = delete;

	template <PlatformType pt>
	static int Configure();

	template <PlatformType pt, typename ResourceType>
	static std::shared_ptr<ResourceType> LoadResource(
		const std::string& configPath);

	template <PlatformType pt>
	static void RefreshResourcesList();

private:
	static OCStackResult SetPlatformInfo(OCPlatformInfo& platformInfo, std::string platformID, std::string manufacturerName,
        std::string manufacturerUrl, std::string modelNumber, std::string dateOfManufacture,
        std::string platformVersion, std::string operatingSystemVersion,
        std::string hardwareVersion, std::string firmwareVersion, std::string supportUrl,
        std::string systemTime);

	static void DuplicateString(char ** targetString, std::string sourceString);

	static constexpr const char* SVR_DB_FILE_NAME = "./oic_svr_db_server.dat";

	static FILE* client_open(const char* /*path*/, const char *mode) {
	    return fopen(SVR_DB_FILE_NAME, mode);
	}

};


struct ResourceInfo {
    ResourceInfo() = default;

    virtual bool fromJson(const std::string& jsonString) {
        const cJSON* name;
        const cJSON* uri;
        const cJSON* type;
        const cJSON* iface;

        const char* c_jsonString = jsonString.c_str();

        jsonObj = cJSON_Parse(c_jsonString);
        if (!jsonObj) {
            return false;
        }

        name = cJSON_GetObjectItemCaseSensitive(jsonObj, "name");
        if (!name) {
            return false;
        }

        uri = cJSON_GetObjectItemCaseSensitive(jsonObj, "uri");
        if (!uri) {
            return false;
        }

        type = cJSON_GetObjectItemCaseSensitive(jsonObj, "type");
        if (!type) {
            return false;
        }

        iface = cJSON_GetObjectItemCaseSensitive(jsonObj, "iface");
        if (!iface) {
            return false;
        }

        mName = name->valuestring;
        mUri = uri->valuestring;
        mType = type->valuestring;
        mIface = iface->valuestring;

        return true;
    }

    const cJSON* jsonObj;

    std::string mName;
    std::string mUri;
    std::string mType;
    std::string mIface;

};

struct ResourceInfoClient: public ResourceInfo {
    virtual bool fromJson(const std::string& jsonString) override {
        const cJSON* async;

        ResourceInfo::fromJson(jsonString);

        async = cJSON_GetObjectItemCaseSensitive(jsonObj, "async");
        if (!cJSON_IsBool(async)) {
            return false;
        }

        mAsync = cJSON_IsTrue(async) ? true : false; 

        return true;
    }

    bool mAsync;

};

struct ResourceInfoServer: public ResourceInfo {
    virtual bool fromJson(const std::string& jsonString) override {
        const cJSON* halPath;

		ResourceInfo::fromJson(jsonString);

        halPath = cJSON_GetObjectItemCaseSensitive(jsonObj, "hal");
        if (!halPath) {
            return false;
        }

        mHalPath = halPath->valuestring;

        return true;
    }

    std::string mHalPath;

};

#endif // SHOUSE_PLATFORM_H
