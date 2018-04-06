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

	static int mState;

	int HAL_open() {
		return 0;
	}

    ShouseHALResult HAL_close(int id) {
        return ShouseHALResult::OK;
    }
    
    ShouseHALResult HAL_get(int id, const std::string& propName, std::string& resultValue, const OC::QueryParamsMap& params) {
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

    ShouseHALResult HAL_put(int id, const std::string& propName, const std::string& newValue, const OC::QueryParamsMap& params) {
        ShouseHALResult ret = ShouseHALResult::OK;

        //curRepr.getValue("state", mState);
        Log::info(LOG_TAG, "Put something: {}", mState);

        return ret;        
    }

    std::vector<ResourceProperty> HAL_properties() {
        // std::string props = "{ \"name\": \"lightness\", \"type\": \"string\", \"default_value\": \"2\"}";

        ResourceProperty prop;
        prop.mName = "lightness";
        prop.mType = ResourceProperty::Type::T_STRING;
        prop.mDefaultValue = "2";

        std::vector<ResourceProperty> vec{prop};
        return vec;
    }	
}