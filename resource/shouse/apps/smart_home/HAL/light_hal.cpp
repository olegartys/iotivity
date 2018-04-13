#include <vector>
#include <string>
#include <atomic>
#include <functional>
#include <algorithm>

#include <OCPlatform.h>
#include <OCApi.h>

#include <shouse/Log.h>

#include <dynamic_resource/resource_property.h>

#include <HAL/shouse_res_hal.h>
#include <HAL/shouse_res_hal_dll.h>

constexpr size_t LIGHTS_COUNT = 2;

extern "C" {
	static constexpr const char* LOG_TAG = "LightHalDll";

    struct Light {
        int mState;
        std::atomic_int mLightness;
        std::string mSomeParam;
    };

    static std::array<Light, LIGHTS_COUNT> gLights;
    static std::array<bool, LIGHTS_COUNT> gBusyLights = {0};

	int HAL_open() {
        auto it = std::find(gBusyLights.begin(), gBusyLights.end(), 0);
        if (it == gBusyLights.end()) {
            return -1;
        }

        *it = 1;

		return std::distance(gBusyLights.begin(), it);
	}

    ShouseHALResult HAL_close(int id) {
        gBusyLights[id] = 0;
        return ShouseHALResult::OK;
    }
    
    ShouseHALResult HAL_get(int id, const std::string& propName, std::string& resultValue, const OC::QueryParamsMap& params) {
        ShouseHALResult ret = ShouseHALResult::OK;

        Log::info(LOG_TAG, "Get something[{}]: {}", id, propName); 

        if (propName == "lightness") {
            resultValue = std::to_string(gLights[id].mLightness);
        } else if (propName == "state") {
            resultValue = std::to_string(gLights[id].mState);
        } else if (propName == "some_param") {
            resultValue = gLights[id].mSomeParam;
        } else {
            ret = ShouseHALResult::ERR;
        }

        return ret;
    }

    ShouseHALResult HAL_put(int id, const std::string& propName,
        const std::string& newValue, const OC::QueryParamsMap& params) {
        ShouseHALResult ret = ShouseHALResult::OK;

        Log::info(LOG_TAG, "Put something[{}]: {}, {}",
            id, propName, newValue);

        if (propName == "lightness") {
            gLights[id].mLightness = std::stoi(newValue);
        } else if (propName == "state") {
            gLights[id].mState = std::stoi(newValue);
        } else if (propName == "some_param") {
            gLights[id].mSomeParam = newValue;
        } else {
            ret = ShouseHALResult::ERR;
        }

        return ret;        
    }

    void HAL_observe(int id, const OC::QueryParamsMap& params,
        std::atomic_bool& isThreadRunning,
        std::function<void(int)> notifyChanges) {
        int ret = 0;

        while (isThreadRunning) {
            gLights[id].mLightness += id + 1;
            Log::debug(LOG_TAG, "Observing[{}]: gLightness = {}",
                id, gLights[id].mLightness);
            notifyChanges(ret);
            sleep(1);
        }
    }

    std::vector<ResourceProperty> HAL_properties() {
        ResourceProperty prop;
        ResourceProperty prop1;
        ResourceProperty prop2;

        prop.mName = "state";
        prop.mType = ResourceProperty::Type::T_INT;
        prop.mValue = "-1";

        prop1.mName = "lightness";
        prop1.mType = ResourceProperty::Type::T_INT;
        prop1.mValue = "-1";

        prop2.mName = "some_param";
        prop2.mType = ResourceProperty::Type::T_STRING;
        prop2.mValue = "tratata";

        std::vector<ResourceProperty> vec{prop, prop1, prop2};
        return vec;
    }	
}
