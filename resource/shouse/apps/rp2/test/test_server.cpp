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

static const char* LOG_TAG = "test_rp2_server";

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Not Enough Arguments: Need Diod Hal path and Motion Sensor Hal path\n");
        exit(EXIT_FAILURE);
    }
    ShouseDefaultPlatform::Configure<PlatformType::SHOUSE_SERVER>();

#ifdef STATIC_HAL
    // Use HAL implemented at compile time

#else
    // Use HAL from shared library

    /* Create diod resource */
    ShouseServerHALdll* dllDiodHal = new ShouseServerHALdll;
    const char* HAL_path = argv[1];
    if (!dllDiodHal->init(HAL_path)) {
        Log::error(LOG_TAG, "Error loading DLL hal from {}", HAL_path);
        return -1;
    }

    ShouseResourceServer diodServerDll("diod", "/a/diod", "type", "iface");
    if (!diodServerDll.createResource(dllDiodHal)) {
        Log::error(LOG_TAG, "Error creating resource from DLL");
    }

    /* Create motion sensor resource */
    ShouseServerHALdll* dllMotionSensorHal = new ShouseServerHALdll;
    const char* HAL_path_motion = argv[2];
    if (!dllMotionSensorHal->init(HAL_path_motion)) {
        Log::error(LOG_TAG, "Error loading DLL hal from {}", HAL_path_motion);
        return -1;
    }

    ShouseResourceServer motionSensorServerDll("motion", "/a/motion", "type", "iface");
    if (!motionSensorServerDll.createResource(dllMotionSensorHal)) {
        Log::error(LOG_TAG, "Error creating camera resource");
    }

#endif

    // Start listen
    Log::info(LOG_TAG, "Listening...");
    while(1);

    return 0;
}
