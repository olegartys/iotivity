#include <vector>
#include <memory>

#include <dirent.h>
#include <cerrno>

#include <shouse_res_client.h>
#include <shouse_res_srv.h>

#include <shouse_default_platform.h>

static constexpr const char* RESOURCES_DIR = "/home/olegartys/src/iotivity/resource/shouse/apps/smart_home/resources";
static constexpr const char* LOG_TAG = "smart_home_client";

using ResourcesPtrList = std::vector<std::shared_ptr<ShouseResourceClient>>;

static int loadResources(const std::string& dirPath,
	ResourcesPtrList& resourcesList) {
    struct dirent *entry;
    DIR *dp;
    const char* path = dirPath.c_str();

    dp = opendir(path);
    if (dp == NULL) {
        Log::error(LOG_TAG, "{}", strerror(errno));
        return -1;
    }

    while ((entry = readdir(dp))) {
    	std::stringstream ss;
    	ss << RESOURCES_DIR << "/" << entry->d_name;
    	auto res =
    		ShouseDefaultPlatform::LoadResource
    		<PlatformType::SHOUSE_CLIENT, ShouseResourceClient>
			(ss.str());
		if (res) {
    		resourcesList.push_back(res);			
		} else {
			Log::error(LOG_TAG, "Error loading resource from {}",
				entry->d_name);
		}
    }

    closedir(dp);

    return 0;
}

static void monitorResources(const ResourcesPtrList& resourcesList) {
	while (1) {
		for (const auto& res: resourcesList) {
			std::cout << to_string(*res) << '\n'; 
		}

		sleep(1);

		system("clear"); // oh, God
	}
}

int main() {
	ShouseDefaultPlatform::Configure<PlatformType::SHOUSE_CLIENT>();

	ResourcesPtrList resources;

	if (loadResources(RESOURCES_DIR, resources) < 0) {
		return -1;
	}

	for (const auto& res: resources) {
		res->startObserve();
	}

	monitorResources(resources);

#if 0
	std::mutex blocker;
	std::condition_variable cv;
	std::unique_lock<std::mutex> lock(blocker);
	cv.wait(lock);
#endif

	return 0;
}