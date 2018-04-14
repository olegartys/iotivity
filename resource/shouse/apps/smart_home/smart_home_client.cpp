#include <vector>
#include <memory>

#include <dirent.h>
#include <cerrno>

#include <shouse_res_client.h>
#include <shouse_res_srv.h>

#include <shouse_default_platform.h>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include <base64.h>

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

/**** CAMERA ROUTINES *****/
static const std::shared_ptr<uint8_t> decodeBase64(const std::string& base64str,
    size_t& outSize) {
    // Get the size of output buffer
    size_t outBufFakeSize = 0;
    size_t outBufRealSize = 0;
    int ret = b64Decode(base64str.c_str(), base64str.length(),
        (uint8_t*)base64str.c_str(), (size_t)outBufFakeSize, &outBufRealSize);
    if (ret != B64_OUTPUT_BUFFER_TOO_SMALL) {
        Log::error(LOG_TAG, "{}: Geting output buffer size failed: {}", __FUNCTION__, ret);
        return nullptr;
    }

    Log::debug(LOG_TAG, "Decode buffer size: {}", outBufRealSize);

    // Decode buffer
    std::shared_ptr<uint8_t> outBuf(new uint8_t[outBufRealSize]);
    ret = b64Decode(base64str.c_str(), base64str.length(), outBuf.get(),
    	(size_t)outBufRealSize, &outBufRealSize);
    if (ret != B64_OK) {
        Log::error(LOG_TAG, "Decode64 failed {}", ret);
        return nullptr;
    }

    outSize = outBufRealSize;

    return outBuf;
}

static void displayFrame(const OC::HeaderOptions&,
    const std::map<std::string, ResourceProperty>& props, const int eCode) {
    size_t outSize{};

    auto decodeFrame = decodeBase64(props.at("frame").mValue, outSize);
    cv::_InputArray arr(decodeFrame.get(), (int)outSize);

    cv::Mat imgMat = cv::imdecode(arr, CV_LOAD_IMAGE_COLOR);
    cv::imshow("kek", imgMat);
    cv::waitKey(20);
}

/***********/

int main() {
	ShouseDefaultPlatform::Configure<PlatformType::SHOUSE_CLIENT>();

	Log::setLevel(spdlog::level::err);
	// Log::setOutputFile("client.log");

	ResourcesPtrList resources;

	if (loadResources(RESOURCES_DIR, resources) < 0) {
		return -1;
	}

	for (const auto& res: resources) {
		if (res->type() != "camera")
			res->startObserve();
	}

	// Deal with camera

	// for (const auto& res: resources) {
	// 	if (res->type() == "camera") {
	// 		while (1) {
	// 			res->get(/*displayFrame*/);
	// 		}
	// 	}
	// }

	monitorResources(resources);

#if 0
	std::mutex blocker;
	std::condition_variable cv;
	std::unique_lock<std::mutex> lock(blocker);
	cv.wait(lock);
#endif

	return 0;
}