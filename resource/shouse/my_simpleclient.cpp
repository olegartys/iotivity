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

#include "baseresource.h"
#include "lightresource.h"
#include "binarylightclient.h"

using namespace OC;
namespace PH = std::placeholders;

static const char* SVR_DB_FILE_NAME = "./oic_svr_db_client.dat";
static const char* LOG_TAG = "my_simpleclient";

static FILE* client_open(const char* /*path*/, const char *mode)
{
    return fopen(SVR_DB_FILE_NAME, mode);
}

std::shared_ptr<BinaryLightClient> pBinaryLight(nullptr);

void onFoundResource(std::shared_ptr<OCResource> resource) {
    Log::debug(LOG_TAG, "{}: resource found {}", __FUNCTION__, resource->uri());

    if (resource->uri() == "/a/light") {

        pBinaryLight = std::make_shared<BinaryLightClient>();
        QueryParamsMap test;
        Log::debug(LOG_TAG, "{}: GET for /a/light", __FUNCTION__);
        auto onGet = std::bind(&BinaryLightClient::onGet, pBinaryLight.get(), PH::_1, PH::_2, PH::_3);
        resource->get(test, onGet);

        while (1) {
            std::cout << "HEREMAIN: " << std::this_thread::get_id() << std::endl;
            auto onGet = std::bind(&BinaryLightClient::onGet, pBinaryLight.get(), PH::_1, PH::_2, PH::_3);
            resource->get(test, onGet);
            sleep(2);
//            pBinaryLight->enable();
//            Log::debug(LOG_TAG, "{}: enable light, PUT for /a/light", __FUNCTION__);
//            auto onPut = std::bind(&BinaryLightClient::onPut, pBinaryLight.get(), PH::_1, PH::_2, PH::_3);
//            resource->put(pBinaryLight->type(), pBinaryLight->iface(), pBinaryLight->repr(), test, onPut);

//            sleep(3);

//            pBinaryLight->disable();
//            Log::debug(LOG_TAG, "{}: disable light, PUT for /a/light", __FUNCTION__);
//            resource->put(pBinaryLight->type(), pBinaryLight->iface(), pBinaryLight->repr(), test, onPut);

//            sleep(5);
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
