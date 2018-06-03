#include <stdio.h>
#include <unistd.h> //usleep

#include <bcm2835.h>

#include <shouse/Log.h>
#include <HAL/shouse_res_hal.h>

#include <vector>
#include <string>
#include <atomic>
#include <functional>

#define POWER_LINE_0 RPI_V2_GPIO_P1_31
#define POWER_LINE_1 RPI_V2_GPIO_P1_33
#define POWER_LINE_2 RPI_V2_GPIO_P1_35
#define POWER_LINE_3 RPI_V2_GPIO_P1_37

#define INPUT_LINE_0 RPI_V2_GPIO_P1_11
#define INPUT_LINE_1 RPI_V2_GPIO_P1_13
#define INPUT_LINE_2 RPI_V2_GPIO_P1_15

#define UNUSED_PARAMETER(x) (void)(x)

extern "C" {

	static constexpr const char* LOG_TAG = "NumPadHall";

	enum { INPUT_LINES = 3,
	       OUTPUT_LINES = 4
	}; // Compile Time Constants

	static const int gOutputLines[OUTPUT_LINES] = {POWER_LINE_0, POWER_LINE_1, POWER_LINE_2, POWER_LINE_3};
	static const int gInputLines[INPUT_LINES] = { INPUT_LINE_0, INPUT_LINE_1, INPUT_LINE_2 };

	static const char gSymbols[OUTPUT_LINES][INPUT_LINES] =
	                                {{'1', '2', '3'},
	                                 {'4', '5', '6'},
	                                 {'7', '8', '9'},
	                                 {'*', '0', '#'}};
	static bool gIsInited = false;
	static char gObserveBuff = 0;

	int init_gpio();

	int HAL_open() {
		if (!bcm2835_init()) {
			return -1;
		}
		init_gpio();
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
		if (gObserveBuff != 0) {
                	char buff[8];
			buff[1] = 0;
			buff[0] = gObserveBuff;
			std::string str(buff);
			resultValue = str;
			return ret;
		}
		char value = 0;
        	for (int i = 0; i < OUTPUT_LINES; ++i) {
                	bcm2835_gpio_write(gOutputLines[i], HIGH);
                	for (int j = 0; j < INPUT_LINES; ++j) {
                        	int lev = bcm2835_gpio_lev(gInputLines[j]);
                        	if (lev == HIGH) {
                        		value = gSymbols[i][j];
				}
	                }
        	        bcm2835_gpio_write(gOutputLines[i], LOW);
                	usleep(1000); //sleep 1ms

        	}
                char buff[8];
                buff[1] = 0;
                buff[0] = value;
                std::string str(buff);
		resultValue = str;

		return ret;
	}

	void HAL_observe(int id, const OC::QueryParamsMap& params,
				std::atomic_bool& isThreadRunning,
				std::function<void(int)> notifyChanges) {

		UNUSED_PARAMETER(id);
		if (!gIsInited) return;

		while(isThreadRunning) {
	        for (int i = 0; i < OUTPUT_LINES; ++i) {
	                bcm2835_gpio_write(gOutputLines[i], HIGH);
	                for (int j = 0; j < INPUT_LINES; ++j) {
	                        int lev = bcm2835_gpio_lev(gInputLines[j]);
	                        if (lev == HIGH) {
                    			gObserveBuff = gSymbols[i][j];
					notifyChanges(0);
		                        gObserveBuff = 0;
					while(lev == HIGH) {
						// wait for the user release the button
		                                lev = bcm2835_gpio_lev(gInputLines[j]);
	                        	}
				}
	                }
	                bcm2835_gpio_write(gOutputLines[i], LOW);
	                usleep(1000); //sleep 1ms

	        } //for
		} //while(isThreadRunning)
	}

	std::vector<ResourceProperty> HAL_properties() {
		ResourceProperty prop;
		prop.mName = "value";
		prop.mType = ResourceProperty::Type::T_STRING;
		prop.mValue = ".";

		std::vector<ResourceProperty> vec {prop};
		return vec;
	}


	int init_gpio() {
	        for (int i = 0; i < OUTPUT_LINES; ++i) {
	                bcm2835_gpio_fsel(gOutputLines[i], BCM2835_GPIO_FSEL_OUTP);
	                bcm2835_gpio_write(gOutputLines[i], LOW);
	     	}
	        for (int i = 0; i < INPUT_LINES; ++i) {
	                bcm2835_gpio_fsel(gInputLines[i], BCM2835_GPIO_FSEL_INPT);
	        }

	        return 0;
	}

}
