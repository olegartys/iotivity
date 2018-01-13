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

///
/// This sample provides steps to define an interface for a resource
/// (properties and methods) and host this resource on the server.
///
#include "iotivity_config.h"

#include <functional>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#include <mutex>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include <shouse_res_srv.h>
#include <shouse_default_platform.h>

#include <HAL/shouse_res_hal.h>

#include <shouse/baseresource.h>
#include <shouse/Log.h>

#include "ocpayload.h"

using namespace OC;
using namespace std;

static const char* LOG_TAG = "test_server";


class LightHAL : public ShouseResHAL {
public:
    int onGet(OCRepresentation& curRepr, const QueryParamsMap& params) noexcept override {
        int ret = 0;

        mState++;
        Log::info(LOG_TAG, "Get something: {}", curRepr.getValue<int>("state"));

        curRepr.setValue("state", mState);

        return ret;
    }

    int onPut(OCRepresentation& curRepr, const QueryParamsMap& params) noexcept override {
        int ret = 0;

        curRepr.getValue("state", mState);
        Log::info(LOG_TAG, "Put something: {}", mState);

        return ret;        
    }

private:
    int mState = 0;

};

int main(int argc, char** argv) {
    ShouseDefaultPlatform::Configure<PlatformType::SHOUSE_SERVER>();

    LightHAL* lightHal = new LightHAL;

    ShouseResourceServer lightServer("/a/light", "type", "iface", lightHal);
    lightServer.createResource();

    // Start listen
    Log::info(LOG_TAG, "Listening...");
    while(1);

    return 0;
}
