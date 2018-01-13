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
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"

#include <shouse_res_client.h>

#include <HAL/shouse_res_hal.h>

#include <shouse/baseresource.h>
#include <shouse/Log.h>
#include <shouse_default_platform.h>

using namespace OC;
namespace PH = std::placeholders;

static const char* LOG_TAG = "my_simpleclient";

class LightHAL : public ShouseClientHAL {
public:
    virtual void onGet(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode) override {
        Log::info(LOG_TAG, "GET returned to client: {}", rep.getValue<int>("state"));

        // Do something with new data
        // Update UI, or something else
    }

    virtual void onPut(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode) override {
        Log::info(LOG_TAG, "PUT returned to client");

        // Do something with new data
        // Update UI, or something else
    }

};

ShouseResourceClient *lightClient;

void onFoundResource(std::shared_ptr<OCResource> resource) {
    Log::debug(LOG_TAG, "{}: resource found {}", __FUNCTION__, resource->uri());

    if (resource->uri() == "/a/light") {
        ShouseClientHAL *hal = new LightHAL;
        lightClient = new ShouseResourceClient("/a/light", "t", "iface", hal);
        lightClient->setResource(resource);

        QueryParamsMap test;
        lightClient->get(test);

        sleep(1);

        auto state = lightClient->repr().getValue<int>("state");

        Log::debug(LOG_TAG, "Current state: {}", state);

        state++;
        lightClient->repr().setValue("state", state);

        lightClient->put(test);
    }
}

void onFoundResourceError(const std::string& err, const int err_t) {
    Log::error(LOG_TAG, "{}: error {}, {}", __FUNCTION__, err, err_t);
}

int main(int argc, char** argv) {
    ShouseDefaultPlatform::Configure<PlatformType::SHOUSE_CLIENT>();

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
