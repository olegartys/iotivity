#ifndef SHOUSE_PLATFORM_H
#define SHOUSE_PLATFORM_H

#include <string>
#include <cstdio>

#include <OCApi.h>
#include <OCPlatform.h>

#include <shouse/Log.h>

enum class PlatformType {
	SHOUSE_CLIENT, SHOUSE_SERVER
};

class ShouseDefaultPlatform final {
public:
	ShouseDefaultPlatform() = delete;

	template <PlatformType pt>
	static int Configure();

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

	static constexpr const char* LOG_TAG = "ShouseDefaultPlatform";

};

#endif // SHOUSE_PLATFORM_H