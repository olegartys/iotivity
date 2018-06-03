#include <vector>
#include <memory>

#include <dirent.h>
#include <cerrno>

#include <shouse_res_client.h>
#include <shouse_res_srv.h>

#include <shouse_default_platform.h>

static constexpr const char* RESOURCES_DIR = "resources";
static constexpr const char* LOG_TAG = "test1";

using ResourcesPtrList = std::vector<std::shared_ptr<ShouseResourceServer>>;

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
	ShouseDefaultPlatform::Configure<PlatformType::SHOUSE_SERVER>();
	
	ResourcesPtrList resources;

	auto res =
    		ShouseDefaultPlatform::LoadResource
    		<PlatformType::SHOUSE_SERVER, ShouseResourceServer>
			("light.json");

	resources.push_back(res);

	// monitorResources(resources);

#if 1
	std::mutex blocker;
	std::condition_variable cv;
	std::unique_lock<std::mutex> lock(blocker);
	cv.wait(lock);
#endif

	return 0;
}
