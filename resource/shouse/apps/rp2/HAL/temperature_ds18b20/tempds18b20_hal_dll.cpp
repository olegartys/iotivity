#include <stdio.h>
#include <unistd.h> //usleep

#include <bcm2835.h>

#include <shouse/Log.h>
#include <HAL/shouse_res_hal.h>

#include <vector>
#include <string>
#include <atomic>
#include <functional>

#define UNUSED_PARAMETER(x) (void)(x)

#define TEMP_PIN RPI_V2_GPIO_P1_07 // BCM2835_GPIO 4


extern "C" {


	int presence(uint8_t pin);
	uint8_t readBit(uint8_t pin);
	int readByte(uint8_t pin);
	void writeBit(uint8_t pin,int b);
	void writeByte(uint8_t pin, int byte);
	int convert(uint8_t pin);
	float getTemperature(uint8_t pin);

	static constexpr const char* LOG_TAG = "TemperatureDS18B20_HAL";

	static bool gIsInited = false;

	int HAL_open() {
		if (!bcm2835_init()) {
			return -1;
		}
		if (presence(TEMP_PIN) == 1) {
			return -2;
		}
		gIsInited = true;

		return 0;
	}

	ShouseHALResult HAL_close(int id) {
		UNUSED_PARAMETER(id);
		bcm2835_close();
		return ShouseHALResult::OK;
	}



	ShouseHALResult HAL_put(int id, const std::string& propName, const std::string& newValue,
								     const OC::QueryParamsMap& params) {
		UNUSED_PARAMETER(id);
                UNUSED_PARAMETER(propName);
                UNUSED_PARAMETER(newValue);
                UNUSED_PARAMETER(params);

		return ShouseHALResult::OK;
	}


	ShouseHALResult HAL_get(int id, const std::string& propName, std::string& resultValue,
                                                                     const OC::QueryParamsMap& params) {
		if (!gIsInited || propName != "value") {
			return ShouseHALResult::ERR;
		}

		ShouseHALResult ret = ShouseHALResult::OK;

		float temp = getTemperature(TEMP_PIN);
		if (temp == -1000) {
			ret = ShouseHALResult::ERR;
		} else {
			resultValue = std::to_string(temp) + " C";
		}
		return ret;
	}

	void HAL_observe(int id, const OC::QueryParamsMap& params,
				std::atomic_bool& isThreadRunning,
				std::function<void(int)> notifyChanges) {

		UNUSED_PARAMETER(id);
		if (!gIsInited) return;
		float lastTemp = getTemperature(TEMP_PIN);
		while(isThreadRunning) {
			float temp = getTemperature(TEMP_PIN);
			if (lastTemp - temp > 0.5 || temp - lastTemp > 0.5) {
				lastTemp = temp;
				notifyChanges(0);
			}

		} //while(isThreadRunning)
	}

	std::vector<ResourceProperty> HAL_properties() {
		ResourceProperty prop;
		prop.mName = "value";
		prop.mType = ResourceProperty::Type::T_STRING;
		prop.mValue = std::to_string(gIsInited ? getTemperature(TEMP_PIN) : -9999);

		std::vector<ResourceProperty> vec {prop};
		return vec;
	}


	uint8_t readBit(uint8_t pin) {
   	    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
	    bcm2835_gpio_write(pin, LOW);
	    bcm2835_delayMicroseconds(6);
	    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
	    bcm2835_delayMicroseconds(8);
	    uint8_t b = bcm2835_gpio_lev(pin);
	    bcm2835_delayMicroseconds(55);
	    return b;
	}

	int readByte(uint8_t pin) {
	    int byte = 0;
	    int i;
	    for (i = 0; i < 8; i++) {
	        byte = byte | readBit(pin) << i;
	    };
	    return byte;
	}

	int presence(uint8_t pin) {
	    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
	    bcm2835_gpio_write(pin, LOW);
	    bcm2835_delayMicroseconds(480);
	    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
	    bcm2835_delayMicroseconds(70);
	    uint8_t b = bcm2835_gpio_lev(pin);
	    bcm2835_delayMicroseconds(410);
	    return b;
	}

	void writeBit(uint8_t pin,int b) {
	    int delay1, delay2;
	    if (b == 1) {
	        delay1 = 6;
	        delay2 = 64;
	    } else {
	        delay1 = 80;
	        delay2 = 10;
	    }
	    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
	    bcm2835_gpio_write(pin, LOW);
	    bcm2835_delayMicroseconds(delay1);
	    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
	    bcm2835_delayMicroseconds(delay2);
	}

	void writeByte(uint8_t pin,int byte) {
	    int i;
	    for (i = 0; i < 8; i++) {
	        if (byte & 1) {
	            writeBit(pin,1);
	        } else {
	            writeBit(pin,0);
	        }
	        byte = byte >> 1;
	    }
	}

	int convert(uint8_t pin) {
	    int i;
	    writeByte(pin, 0x44);
	    for (i = 0; i < 1000; i++) {
	        bcm2835_delayMicroseconds(100000);
	        if (readBit(pin) == 1)break;
	    }
	    return i;
	}

	float getTemperature(uint8_t pin) {
	    if (presence(pin) == 1) return -1000;
	    writeByte(pin, 0xCC);
	    convert(pin);
	    presence(pin);
	    writeByte(pin, 0xCC);
	    writeByte(pin, 0xBE);
	    int i;
	    uint8_t data[9];
	    for (i = 0; i < 9; i++) {
	        data[i] = readByte(pin);
	    }
	    int t1 = data[0];
	    int t2 = data[1];
	    int16_t temp1 = (t2 << 8 | t1);
	    float temp = (float) temp1 / 16;
	    return temp;
	}

}
