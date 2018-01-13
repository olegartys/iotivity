#include "iotivity_config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <Windows.h>
#endif
#include <string>
#include <map>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include "OCPlatform.h"
#include "OCApi.h"

#include "videocameraclient.h"
#include "Log.h"

using namespace OC;
namespace PH = std::placeholders;

static const char* SVR_DB_FILE_NAME = "./oic_svr_db_client.dat";
static const char* LOG_TAG = "my_simpleclient";

std::mutex gMutex;
std::condition_variable gCondVar;

static FILE* client_open(const char* /*path*/, const char *mode)
{
    return fopen(SVR_DB_FILE_NAME, mode);
}

std::shared_ptr<VideoCameraClient> pVideoCam(nullptr);

void onFoundResource(std::shared_ptr<OCResource> resource) {
    Log::debug(LOG_TAG, "{}: resource found {}", __FUNCTION__, resource->uri());

    if (resource->uri() == "/a/video") {
        pVideoCam = std::make_shared<VideoCameraClient>();
        QueryParamsMap test;
        Log::debug(LOG_TAG, "{}: GET for /a/video", __FUNCTION__);

        auto onGet = std::bind(&VideoCameraClient::onGet, pVideoCam.get(), PH::_1, PH::_2, PH::_3);
        while (1) {
            resource->get(test, onGet);
            std::unique_lock<std::mutex> lock(gMutex);
            gCondVar.wait(lock);
        }
    }

}

void onFoundResourceError(const std::string& err, const int err_t) {
    Log::error(LOG_TAG, "{}: error {}, {}", __FUNCTION__, err, err_t);
}

int main(int argc, char** argv) {
    OCPersistentStorage ps {client_open, fread, fwrite, fclose, unlink };
    // Create PlatformConfig object
    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Both,
        "0.0.0.0",
        0,
        OC::QualityOfService::HighQos,
        &ps
    };

    OCPlatform::Configure(cfg);

    // Finding resources
    std::stringstream requestURI;
    requestURI << OC_RSRVD_WELL_KNOWN_URI; //"/a/light";
    Log::debug(LOG_TAG, "Discovering URI: {}", requestURI.str());

    OCPlatform::findResource("", requestURI.str(), CT_DEFAULT, onFoundResource,
                             onFoundResourceError);

    // A condition variable will free the mutex it is given, then do a non-
    // intensive block until 'notify' is called on it.  In this case, since we
    // don't ever call cv.notify, this should be a non-processor intensive version
    // of while(true);
    Log::info(LOG_TAG, "Waiting...");
    std::mutex blocker;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock(blocker);
    cv.wait(lock);

    return 0;
}
