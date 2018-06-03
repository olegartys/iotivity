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

#define I2C_WRITE_ADDRESS 0xF7
#define I2C_READ_ADDRESS  0x77

extern "C" {
        void getPressureAndTemperature(long* pressure, long* temperature);

	static constexpr const char* LOG_TAG = "PressureBMP180_HAL";
	static bool gIsInited = false;

	int HAL_open() {
		if (!bcm2835_init()) {
			return -1;
		}
	        if (!bcm2835_i2c_begin()) {
			return -2;
	        }

	        bcm2835_i2c_setClockDivider(BCM2835_I2C_CLOCK_DIVIDER_2500);
	        bcm2835_i2c_set_baudrate(1000);

		gIsInited = true;

		return 0;
	}

	ShouseHALResult HAL_close(int id) {
		UNUSED_PARAMETER(id);
		bcm2835_i2c_end();
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
		if (!gIsInited) {
			return ShouseHALResult::ERR;
		}

		ShouseHALResult ret = ShouseHALResult::OK;

                long pressure;
                long temperature;
                getPressureAndTemperature(&pressure, &temperature);

		if (propName == "temperature_bmp180") {
			resultValue = std::to_string(temperature) + " C";
		} else if (propName == "pressure") {
			resultValue = std::to_string(pressure) + " Pa";
		} else {
			ret = ShouseHALResult::ERR;
		}

		return ret;
	}

	void HAL_observe(int id, const OC::QueryParamsMap& params,
				std::atomic_bool& isThreadRunning,
				std::function<void(int)> notifyChanges) {

		UNUSED_PARAMETER(id);
		if (!gIsInited) return;
		long lastPressure;
		long lastTemperature;
		getPressureAndTemperature(&lastPressure, &lastTemperature);
		while(isThreadRunning) {
			long pressure;
			long temperature;
	                getPressureAndTemperature(&pressure, &temperature);
			if (lastTemperature - temperature > 0.5 ||
					      temperature - lastTemperature > 0.5) {
				lastTemperature = temperature;
				notifyChanges(0);
			}
			if (lastPressure - pressure > 500 || pressure - lastPressure > 500) {
				lastPressure = pressure;
				notifyChanges(0);
			}

		} //while(isThreadRunning)
	}

	std::vector<ResourceProperty> HAL_properties() {
		ResourceProperty prop, prop1;
		prop.mName = "temperature_bmp180";
		prop.mType = ResourceProperty::Type::T_INT;
		prop.mValue = "0";

		prop1.mName = "pressure";
		prop1.mType = ResourceProperty::Type::T_INT;
		prop1.mValue = "0";

		std::vector<ResourceProperty> vec {prop, prop1};
		return vec;
	}

	void getPressureAndTemperature(long* pressure, long* temperature) {
	        char buff[32];
	        char startAddr = 0xaa;
	        for (int i = 0; i < 22; ++i) {
	                bcm2835_i2c_read_register_rs(&startAddr, buff + i, 1);
	                printf("%d Value: %d%c", i, buff[i], i%2 ? '\n' : '\t');
	                ++startAddr;
	        }

		//get temperature
	        bcm2835_i2c_setSlaveAddress(I2C_WRITE_ADDRESS);
	        buff[0] = 0xF4; buff[1] = 0x2E;
	        bcm2835_i2c_write(buff, 2);
	        usleep(4 * 1000 + 500); // wait 4.5 ms

	        uint16_t temp = 0;

	        char tmpBuff[2] = {0, 0};
	        char addr = 0xf6;
	        bcm2835_i2c_read_register_rs(&addr, tmpBuff, 1);
	        addr = 0xf7;
	        bcm2835_i2c_read_register_rs(&addr, tmpBuff + 1, 1);
	        temp = (tmpBuff[0] << 8) | tmpBuff[1];

///////

	        //get pressure
	        char oss = 3; //over sampling
	        buff[0] = 0xF4; buff[1] = 0x34 + (oss << 6);
	        bcm2835_i2c_write(buff, 2);
	        usleep(25 * 1000 + 500); // wait 25.5 ms

	        char presBuff[3] = {0, 0, 0};
	        addr = 0xf6;
	        bcm2835_i2c_read_register_rs(&addr, presBuff, 1);
	        addr = 0xf7;
	        bcm2835_i2c_read_register_rs(&addr, presBuff + 1, 1);
	        addr = 0xf8;
	        bcm2835_i2c_read_register_rs(&addr, presBuff + 2, 1);

	        long up = ((presBuff[0] << 16) + (presBuff[1] << 8) + presBuff[2]) >> (8 - oss);

		// Calculate real temperature
	        uint16_t ac6 = (buff[10] << 8) + buff[11];
	        uint16_t ac5 = (buff[8] << 8) + buff[9];
	        int16_t mc = (buff[18] << 8) + buff[19];
	        int16_t md = (buff[20] << 8) + buff[21];
	        long x1 = (temp - ac6) * ac5 / pow(2,15);
	        long x2 = mc * pow(2, 11) / (x1 + md);
	        long b5 = x1 + x2;
	        *temperature = (b5 + 8) / pow(2, 4);

		// Calculate real pressure
	        long b6 = b5 - 4000;
	        int16_t b2 = (buff[14] << 8) + buff[15];
	        long px1 = (b2 * (b6 * b6 / pow(2 ,12))) / pow(2,11);
	        int16_t ac2 = (buff[2]  << 8) + buff[3];
	        long px2 = ac2 * b6 / pow(2, 11);
	        long px3 = px2 + px1;
	        int16_t ac1 = (buff[0] << 8) + buff[1];
	        long b3 = (((ac1 * 4 + px3) << oss) + 2) / 4;
	        int16_t ac3 = (buff[4] << 8) + buff[5];
	        px1 = ac3 * b6 / pow(2,13);
	        int16_t b1 = (buff[12] << 8) + buff[13];
	        px2 =(b1 * (b6 * b6 / pow(2,12))) / pow(2,16);
	        px3 = ((px1 + px2) + 2) / pow(2,2);
	        uint16_t ac4 = (buff[6] << 8) + buff[7];
	        unsigned long b4 = ac4 * ((unsigned long)(px3 + 32768)) / pow(2, 15);
	        unsigned long b7 = (((unsigned long)up) - b3) * (50000 >> oss);
	        long p = 0;
	        if (b7 < 0x80000000) {
	                p = (b7 * 2) / b4;
	        } else {
	                p = (b7 / b4) * 2;
	        }
	        px1 = (p / pow(2, 8)) * (p / pow(2,8));
	        px1 = (px1 * 3038) / pow(2,16);
	        px2 = (-7357 * p) / pow(2,16);
	        p = p + ((px1 + px2 + 3791) / pow(2,4));

		*pressure = p;
	}

}
