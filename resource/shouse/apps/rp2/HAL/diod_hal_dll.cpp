#include <bcm2835.h>

#include <shouse/Log.h>
#include <HAL/shouse_res_hal.h>

#include <vector>
#include <string>
#include <atomic>
#include <functional>

#define UNUSED_PARAMETER(x) (void)(x)

#define LED_PIN RPI_V2_GPIO_P1_11

extern "C" {
    static constexpr const char* LOG_TAG = "DiodHalDll";
    static bool isInited = false;

    int HAL_open() {
        if (!bcm2835_init()) {
            return -1;
        }

        return 0;
    }

    ShouseHALResult HAL_close(int id) {
        UNUSED_PARAMETER(id);
        bcm2835_close();
        return ShouseHALResult::OK;
    }
    
    ShouseHALResult HAL_get(int id, const std::string& propName, std::string& resultValue,
                                                                 const OC::QueryParamsMap& params) {
        if (!isInited) {
            return ShouseHALResult::ERR;
        }
        ShouseHALResult ret = ShouseHALResult::OK;

        Log::info(LOG_TAG, "Get something: {}", propName); 

        if (propName == "currState") {
            resultValue = std::to_string(bcm2835_gpio_lev(LED_PIN));
        } else {
            ret = ShouseHALResult::ERR;
        }

        return ret;
    }

    ShouseHALResult HAL_put(int id, const std::string& propName, const std::string& newValue,
                                                                 const OC::QueryParamsMap& params) {
        if (!isInited) {
            return ShouseHALResult::ERR;
        }
        ShouseHALResult ret = ShouseHALResult::OK;

        Log::info(LOG_TAG, "Put something: {}, {}", propName, newValue);
        if (propName == "newState") {
            int val = std::stoi(newValue);
            if (val < 0 && val > 1) {
                ret = ShouseHALResult::ERR;
            } else {
                bcm2835_gpio_write(LED_PIN, val);
            }
        } else {
            ret = ShouseHALResult::ERR;
        }

        return ret;
    }

    void HAL_observe(int id, const OC::QueryParamsMap& params,
        std::atomic_bool& isThreadRunning,
        std::function<void(int)> notifyChanges) {
        UNUSED_PARAMETER(id);
        UNUSED_PARAMETER(params);
        UNUSED_PARAMETER(isThreadRunning);
        UNUSED_PARAMETER(notifyChanges);
        return;
    }

    std::vector<ResourceProperty> HAL_properties() {
        ResourceProperty prop;
        ResourceProperty prop1;

        prop.mName = "currState";
        prop.mType = ResourceProperty::Type::T_INT;

        prop1.mName = "newState";
        prop1.mType = ResourceProperty::Type::T_INT;

        prop.mValue = prop1.mValue = isInited ? bcm2835_gpio_lev(LED_PIN) : LOW;

        std::vector<ResourceProperty> vec {prop, prop1};

        return vec;
    }
}
