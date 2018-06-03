#include <bcm2835.h>

#include <shouse/Log.h>
#include <HAL/shouse_res_hal.h>

#include <vector>
#include <string>
#include <atomic>
#include <functional>

#include "MFRC522.h"

#include <unistd.h> //usleep

#define UNUSED_PARAMETER(x) (void)(x)


extern "C" {
    static constexpr const char* LOG_TAG = "DiodHalDll";

    static  MFRC522 mfrc;
    static std::string cache;
   
    int HAL_open() {
        mfrc.PCD_Init();
        return 0;
    }

    ShouseHALResult HAL_close(int id) {
        UNUSED_PARAMETER(id);
        bcm2835_close();
        return ShouseHALResult::OK;
    }

    ShouseHALResult HAL_get(int id, const std::string& propName, std::string& resultValue,
                                                                 const OC::QueryParamsMap& params) {
        Log::info(LOG_TAG, "Get something: {}", propName);
        ShouseHALResult ret = ShouseHALResult::OK;

        UNUSED_PARAMETER(id);
        UNUSED_PARAMETER(params);


        if (propName == "rfid_value") {
			printf("KEK: %s\n", cache.c_str());
			resultValue = std::string(cache);
			cache = "-";
        } else {

        }

        return ret;
    }

    ShouseHALResult HAL_put(int id, const std::string& propName, const std::string& newValue,
                                                                 const OC::QueryParamsMap& params) {
        ShouseHALResult ret = ShouseHALResult::OK;
        UNUSED_PARAMETER(id);
        UNUSED_PARAMETER(propName);
        UNUSED_PARAMETER(newValue);
        UNUSED_PARAMETER(params);

        return ret;
    }

    void HAL_observe(int id, const OC::QueryParamsMap& params,
        std::atomic_bool& isThreadRunning,
        std::function<void(int)> notifyChanges) {
        UNUSED_PARAMETER(id);
        UNUSED_PARAMETER(params);
        UNUSED_PARAMETER(isThreadRunning);
        printf("start Observe");
        while(isThreadRunning) {
			// Look for a card
			if(!mfrc.PICC_IsNewCardPresent())
			  continue;

			if( !mfrc.PICC_ReadCardSerial())
			  continue;

			// Print UID
			std::stringstream ss;
			for(byte i = 0; i < mfrc.uid.size; ++i){
			  if(mfrc.uid.uidByte[i] < 0x10){
				ss << " 0" << std::hex << (int)(mfrc.uid.uidByte[i]);
				printf(" 0");
				printf("%X",mfrc.uid.uidByte[i]);
			  } else {
				ss << " " << std::hex << (int)(mfrc.uid.uidByte[i]);
				printf(" ");
				printf("%X", mfrc.uid.uidByte[i]);
			  }
		   }
		   cache = ss.str();
		   notifyChanges(0);
		   usleep(1000 * 1000);
		  
		}
	return;
    }

    std::vector<ResourceProperty> HAL_properties() {
        ResourceProperty prop;

        prop.mName = "rfid_value";
        prop.mType = ResourceProperty::Type::T_STRING;
        prop.mValue = "_";

        std::vector<ResourceProperty> vec {prop};

        return vec;
    }
}
