#include <vector>
#include <string>

#include <OCPlatform.h>
#include <OCApi.h>

#include <shouse/Log.h>

#include <dynamic_resource/resource_property.h>

#include <HAL/shouse_res_hal.h>
#include <HAL/shouse_res_hal_dll.h>

extern "C" {
	static constexpr const char* LOG_TAG = "LightHalDll";

	static int gState;
    static int gLightness;
    static std::string gSomeParam;

	int HAL_open() {
        gState = 0;
        gLightness = 0;
        gSomeParam = "hello";
		return 0;
	}

    ShouseHALResult HAL_close(int id) {
        return ShouseHALResult::OK;
    }
    
    ShouseHALResult HAL_get(int id, const std::string& propName, std::string& resultValue, const OC::QueryParamsMap& params) {
        ShouseHALResult ret = ShouseHALResult::OK;

        Log::info(LOG_TAG, "Get something: {}", propName); 

        if (propName == "lightness") {
            resultValue = std::to_string(gLightness);
        } else if (propName == "state") {
            resultValue = std::to_string(gState);
        } else if (propName == "some_param") {
            resultValue = gSomeParam;
        } else {
            ret = ShouseHALResult::ERR;
        }

        return ret;
    }

    ShouseHALResult HAL_put(int id, const std::string& propName, const std::string& newValue, const OC::QueryParamsMap& params) {
        ShouseHALResult ret = ShouseHALResult::OK;

        Log::info(LOG_TAG, "Put something: {}, {}", propName, newValue);

        if (propName == "lightness") {
            gLightness = std::stoi(newValue);
        } else if (propName == "state") {
            gState = std::stoi(newValue);
        } else if (propName == "some_param") {
            gSomeParam = newValue;
        } else {
            ret = ShouseHALResult::ERR;
        }

        return ret;        
    }

    std::vector<ResourceProperty> HAL_properties() {
        ResourceProperty prop;
        ResourceProperty prop1;
        ResourceProperty prop2;

        prop.mName = "state";
        prop.mType = ResourceProperty::Type::T_INT;
        prop.mValue = std::to_string(gState);

        prop1.mName = "lightness";
        prop1.mType = ResourceProperty::Type::T_INT;
        prop1.mValue = std::to_string(gLightness);

        prop2.mName = "some_param";
        prop2.mType = ResourceProperty::Type::T_STRING;
        prop2.mValue = gSomeParam;

        std::vector<ResourceProperty> vec{prop, prop1, prop2};
        return vec;
    }	
}
