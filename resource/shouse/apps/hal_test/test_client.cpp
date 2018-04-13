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

// OCClient.cpp : Defines the entry point for the console application.
//
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
#include <thread>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"

#include <shouse_res_client.h>

#include <HAL/shouse_res_hal.h>

#include <shouse/base_resource.h>
#include <shouse/Log.h>
#include <shouse_default_platform.h>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include "base64.h"

using namespace OC;
namespace PH = std::placeholders;

static const char* LOG_TAG = "my_simpleclient";


class ResourceHolder {
public:
    void addResource(const std::string& uri,
        ShouseResourceClient* resource) {
        std::lock_guard<std::mutex> lock(mLock);

        // Add new resource as a shared pointer. We will always hold one,
        // so the objects will never be destructed and lost

        mResources[uri] =
            std::move(std::shared_ptr<ShouseResourceClient>(resource));

        // Notify getResource that new resource with uri = mRecentUriAdded
        // has been added

        mRecentUriAdded = uri;
        mResourceWaitCv.notify_all();

        Log::info(LOG_TAG, "Resource uri = {} added", uri);
    }

    std::shared_ptr<ShouseResourceClient> getResourceSync(const std::string& uri) {
        
        // Check whether resource with given uri already stored

        {
            std::lock_guard<std::mutex> lock(mLock);
            if (mResources.find(uri) != mResources.end()) {
                // Alright, resource found -> just return pointer

                return mResources[uri];
            }
        }
        
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

    std::map<std::string, std::shared_ptr<ShouseResourceClient>> mResources;
    std::mutex mLock;

    std::condition_variable mResourceWaitCv;

    std::string mRecentUriAdded;

};

ResourceHolder gResourceHolder;


static void onFoundResource(std::shared_ptr<OCResource> resource) {
    Log::debug(LOG_TAG, "{}: resource found {}", __FUNCTION__, resource->uri());

    if (resource->uri() == "/a/light") {
        ShouseResourceClient* lightClient =
            new ShouseResourceClient("light", "/a/light", "t", "iface");
        lightClient->setOCResource(resource);

        gResourceHolder.addResource(resource->uri(), lightClient);
    } else if (resource->uri() == "/a/camera") {
        ShouseResourceClient* cameraClient = 
            new ShouseResourceClient("camera", "/a/camera", "t", "iface");
        cameraClient->setOCResource(resource);

        gResourceHolder.addResource(resource->uri(), cameraClient);
    }
}

static void onFoundResourceError(const std::string& err, const int err_t) {
    Log::error(LOG_TAG, "{}: error {}, {}", __FUNCTION__, err, err_t);
}


static void onGet(const OC::HeaderOptions&,
    const std::map<std::string, ResourceProperty>& props, const int eCode) {
    Log::info(LOG_TAG, "GET returned to client: {}", eCode);

    // Do something with new data
    // Update UI, or something else

    Log::debug(LOG_TAG, "Properties of /a/light: ");
    for (const auto& prop: props) {
        auto p = prop.second;
        Log::debug(LOG_TAG, "name: {}, type: {}, val: {}",
            p.mName, propertyTypeToStr(p.mType), p.mValue);
    }
}

static void onPut(const OC::HeaderOptions&,
    const std::map<std::string, ResourceProperty>& props, const int eCode) {
    Log::info(LOG_TAG, "PUT returned to client: {}", eCode);

    // Do something with new data
    // Update UI, or something else

    Log::debug(LOG_TAG, "Properties of /a/light: ");
    for (const auto& prop: props) {
        auto p = prop.second;
        Log::debug(LOG_TAG, "name: {}, type: {}, val: {}",
            p.mName, propertyTypeToStr(p.mType), p.mValue);
    }
}

static void onObserve(const OC::HeaderOptions&,
        const std::map<std::string, ResourceProperty>& props, const int eCode) {
    Log::info(LOG_TAG, "Observe callback: {}", eCode);

    Log::debug(LOG_TAG, "Properties of /a/light: ");
    for (const auto& prop: props) {
        auto p = prop.second;
        Log::debug(LOG_TAG, "name: {}, type: {}, val: {}",
            p.mName, propertyTypeToStr(p.mType), p.mValue);
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
    ret = b64Decode(base64str.c_str(), base64str.length(), outBuf.get(), (size_t)outBufRealSize, &outBufRealSize);
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

int main(int argc, char** argv) {
    ShouseDefaultPlatform::Configure<PlatformType::SHOUSE_CLIENT>();

    // Finding resources
    std::stringstream requestURI;
    requestURI << OC_RSRVD_WELL_KNOWN_URI; //"/a/light";
    Log::debug(LOG_TAG, "Discovering URI: {}", requestURI.str());

    OCPlatform::findResource("", requestURI.str(), CT_DEFAULT, onFoundResource,
                             onFoundResourceError);

    // Wait for the /a/light to be added

    auto lightResource = gResourceHolder.getResourceSync("/a/light");

    lightResource->startObserve(onObserve);

    lightResource->get(onGet);

    /* This set of setProp calls is in race condition with observation thread.
     */

    lightResource->setProp("lightness", "5");
    lightResource->setProp("state", "1");
    lightResource->setProp("some_param", "pararam");

    lightResource->put(onPut);

    lightResource->get(onGet);

    // sleep(5);

    lightResource->stopObserve();

    // Wait for camera to be added

    auto cameraResource = gResourceHolder.getResourceSync("/a/camera");

    /* It does not work */

    // cameraResource->startObserve(onObserve);

    while (1) {
        cameraResource->get(onGet);
        usleep(33 * 1000);
    }

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
