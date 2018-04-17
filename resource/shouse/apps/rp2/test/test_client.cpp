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

using namespace OC;
namespace PH = std::placeholders;

static const char* LOG_TAG = "simple_rp2_client";


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

    if (resource->uri() == "/a/diod") {
        ShouseResourceClient* diodClient =
            new ShouseResourceClient("diod", "/a/diod", "t", "iface");
        diodClient->setOCResource(resource);

        gResourceHolder.addResource(resource->uri(), diodClient);
    } else if (resource->uri() == "/a/motion") {
        ShouseResourceClient* motionClient = 
            new ShouseResourceClient("motion", "/a/motion", "t", "iface");
        motionClient->setOCResource(resource);

        gResourceHolder.addResource(resource->uri(), motionClient);
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

    Log::debug(LOG_TAG, "Properties of /a/diod: ");
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

    Log::debug(LOG_TAG, "Properties of /a/diod: ");
    for (const auto& prop: props) {
        auto p = prop.second;
        Log::debug(LOG_TAG, "name: {}, type: {}, val: {}",
            p.mName, propertyTypeToStr(p.mType), p.mValue);
    }
}

static void onObserve(const OC::HeaderOptions&,
        const std::map<std::string, ResourceProperty>& props, const int eCode) {
    Log::info(LOG_TAG, "Observe callback: {}", eCode);

    Log::debug(LOG_TAG, "Properties of /a/diod: ");
    for (const auto& prop: props) {
        auto p = prop.second;
        Log::debug(LOG_TAG, "name: {}, type: {}, val: {}",
            p.mName, propertyTypeToStr(p.mType), p.mValue);
    }
}


int main(int argc, char** argv) {
    ShouseDefaultPlatform::Configure<PlatformType::SHOUSE_CLIENT>();

    // Finding resources
    std::stringstream requestURI;
    requestURI << OC_RSRVD_WELL_KNOWN_URI;
    Log::debug(LOG_TAG, "Discovering URI: {}", requestURI.str());

    OCPlatform::findResource("", requestURI.str(), CT_DEFAULT, onFoundResource,
                             onFoundResourceError);

    // Wait for the /a/light to be added

    auto lightResource = gResourceHolder.getResourceSync("/a/diod");

    lightResource->startObserve(onObserve);

    lightResource->get(onGet);

    /* This set of setProp calls is in race condition with observation thread.
     */
    int value = 1;
    while (1) {
	std::string strValue = std::to_string(value);
        lightResource->setProp("state", strValue);

        lightResource->put(onPut);

        lightResource->get(onGet);

	value = value ? 0 : 1;
        sleep(1);
    }
//    lightResource->stopObserve();


    return 0;
}

