#include <bcm2835.h>

#include <shouse/Log.h>
#include <HAL/shouse_res_hal.h>

#include <vector>
#include <string>
#include <atomic>
#include <functional>

#define UNUSED_PARAMETER(x) (void)(x)

#define MOTION_PIN RPI_V2_GPIO_P1_32

extern "C" {
    static constexpr const char* LOG_TAG = "MotionSensorHalDll";
    static bool isInited = false;

    int HAL_open() {
        if (!bcm2835_init()) {
            return -1;
        }
	isInited = true;
        return 0;
    }

    ShouseHALResult HAL_close(int id) {
        (void)id;
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

        if (propName == "state") {
            resultValue = std::to_string(bcm2835_gpio_lev(MOTION_PIN));
        } else {
            ret = ShouseHALResult::ERR;
        }

        return ret;
    }

    ShouseHALResult HAL_put(int id, const std::string& propName, const std::string& newValue,
                                                                 const OC::QueryParamsMap& params) {
        return ShouseHALResult::OK;
    }

    void HAL_observe(int id, const OC::QueryParamsMap& params,
        std::atomic_bool& isThreadRunning,
        std::function<void(int)> notifyChanges) {
        UNUSED_PARAMETER(id);

        int lastState = bcm2835_gpio_lev(MOTION_PIN);
        int state = lastState;

        while(isThreadRunning) {
            state = bcm2835_gpio_lev(MOTION_PIN);
            if (state != lastState) {
                notifyChanges(state);
                lastState = state;
            }
        }
        return;
    }

    std::vector<ResourceProperty> HAL_properties() {
        ResourceProperty prop;

        prop.mName = "state";
        prop.mType = ResourceProperty::Type::T_INT;

        prop.mValue = isInited ? bcm2835_gpio_lev(MOTION_PIN) : LOW;

        std::vector<ResourceProperty> vec{prop};

        return vec;
    }
}
