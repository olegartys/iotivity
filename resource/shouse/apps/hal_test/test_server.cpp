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

#include <vector>

#include <shouse_res_srv.h>
#include <shouse_default_platform.h>

#include <HAL/shouse_res_hal.h>
#include <HAL/shouse_res_hal_dll.h>

#include <shouse/base_resource.h>
#include <shouse/Log.h>


#include <dynamic_resource/dynamic_resource.h>


#include "ocpayload.h"

using namespace OC;
using namespace std;

static const char* LOG_TAG = "test_server";

// #define STATIC_HAL

#ifdef STATIC_HAL
class LightHAL : public ShouseServerHAL {
public:
    int open() {
        return 0;
    }

    ShouseHALResult close(int id) {
        return ShouseHALResult::OK;
    }
    
    ShouseHALResult get(int id, const std::string& propName, std::string& resultValue, const OC::QueryParamsMap& params) override {
        ShouseHALResult ret = ShouseHALResult::OK;

        if (propName == "lightness") {
            Log::info(LOG_TAG, "Get something: {}", propName); 
            mState++;
            resultValue = std::to_string(mState);
        } else {
            ret = ShouseHALResult::ERR;
        }

        return ret;
    }

    ShouseHALResult put(int id, const std::string& propName, const std::string& newValue, const OC::QueryParamsMap& params) override {
        ShouseHALResult ret = ShouseHALResult::OK;

        //curRepr.getValue("state", mState);
        Log::info(LOG_TAG, "Put something: {}", mState);

        return ret;        
    }

    virtual std::vector<ResourceProperty> properties() const override {
        // std::string props = "{ \"name\": \"lightness\", \"type\": \"string\", \"default_value\": \"2\"}";

        ResourceProperty prop;
        prop.mName = "lightness";
        prop.mType = ResourceProperty::Type::T_STRING;
        prop.mValue = "2";

        std::vector<ResourceProperty> vec{prop};
        return vec;
    }

private:
    int mState = 0;

};
#endif

int main(int argc, char** argv) {
    ShouseDefaultPlatform::Configure<PlatformType::SHOUSE_SERVER>();

    // Use HAL implemented at compile time

#ifdef STATIC_HAL
    LightHAL* lightHal = new LightHAL;

    ShouseResourceServer lightServer("/a/light", "type", "iface");
    if (!lightServer.createResource(lightHal)) {
        Log::error(LOG_TAG, "Error creating resource");
    }
#else
    // Use HAL from shared library

    /* Create light resource */

    ShouseServerHALdll* dllLightHal = new ShouseServerHALdll;
    const char* HAL_path = "/home/olegartys/src/iotivity/resource/shouse/apps/hal_test/libHAL_light.so";
    if (!dllLightHal->init(HAL_path)) {
        Log::error(LOG_TAG, "Error loading DLL hal from {}", HAL_path);
        return -1;
    }

    ShouseResourceServer lightServerDll("/a/light", "type", "iface");
    if (!lightServerDll.createResource(dllLightHal)) {
        Log::error(LOG_TAG, "Error creating resource from DLL");
    }

    /* Create video camera resource */

    ShouseServerHALdll* dllCameraHal = new ShouseServerHALdll;
    const char* HAL_path_camera = "/home/olegartys/src/iotivity/resource/shouse/apps/hal_test/libHAL_videocamera.so";
    if (!dllCameraHal->init(HAL_path_camera)) {
        Log::error(LOG_TAG, "Error loading DLL hal from {}", HAL_path_camera);
        return -1;
    }

    ShouseResourceServer cameraServerDll("/a/camera", "type", "iface");
    if (!cameraServerDll.createResource(dllCameraHal)) {
        Log::error(LOG_TAG, "Error creating camera resource");
    }

#endif

    // Start listen
    Log::info(LOG_TAG, "Listening...");
    while(1);

    return 0;
}
