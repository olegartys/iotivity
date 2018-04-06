#pragma once

#include <vector>
#include <cstring>

#include <dlfcn.h>

#include <OCPlatform.h>
#include <OCApi.h>

#include <shouse/Log.h>
#include <HAL/shouse_res_hal.h>


class ShouseServerHALdll : public ShouseServerHAL {
public:
	ShouseServerHALdll() = default;

	/* Simply deprecate copy since it is unnecessary (and rather hard)
	 * to maintain dlopened resource.
	 */

	ShouseServerHALdll(const ShouseServerHALdll&) = delete;
	ShouseServerHALdll& operator= (const ShouseServerHALdll&) = delete;

	bool init(const char *path) {
		mDllHndl = dlopen(path, RTLD_LAZY);
		if (!mDllHndl) {
			return false;
		}

		if (!loadSymbols()) {
			return false;
		}

		return true;
	}

	bool destroy() {
		if (mDllHndl) {
			dlclose(mDllHndl);
		}

		return true;
	}

public:
	int open() override {
		Log::info(LOG_TAG, "Calling HAL_open...");
		return mOpenCb();
	}

	ShouseHALResult close(int id) override {
		return mCloseCb(id);
	}

	ShouseHALResult get(int id, const std::string& propName,
		std::string& resultValue, const OC::QueryParamsMap& params) override {
		return mGetCb(id, propName, resultValue, params);
	}

	ShouseHALResult put(int id, const std::string& propName,
		const std::string& newValue, const OC::QueryParamsMap& params) override {
		return mPutCb(id, propName, newValue, params);
	}

	std::vector<ResourceProperty> properties() const override {
		return mPropertiesCb();
	}

private:
	template <typename CbType>
	CbType loadSymbol(const char* name) {
		return reinterpret_cast<CbType>(dlsym(mDllHndl, name));
	}

	bool checkDllLoadErr(const char* cbName) {
		auto dlsym_error = dlerror();
    	if (dlsym_error) {
    		Log::error(LOG_TAG, "Error loading {} function: {}", cbName,
    			dlsym_error);
    		return true;
		}

		return false;
	}

	bool loadSymbols() {
		mOpenCb = loadSymbol<open_t>("HAL_open");
		if (!mOpenCb) {
			if (!checkDllLoadErr("HAL_open")) {
				Log::error(LOG_TAG, "Error getting dlsym error! :(");				
			}

			return false;
		}

		mCloseCb = loadSymbol<close_t>("HAL_close");
		if (!mCloseCb) {
			if (!checkDllLoadErr("HAL_close")) {
				Log::error(LOG_TAG, "Error getting dlsym error! :(");				
			}

			return false;
		}

		mGetCb = loadSymbol<get_t>("HAL_get");
		if (!mGetCb) {
			if (!checkDllLoadErr("HAL_get")) {
				Log::error(LOG_TAG, "Error getting dlsym error! :(");				
			}

			return false;
		}

		mPutCb = loadSymbol<put_t>("HAL_put");
		if (!mPutCb) {
			if (!checkDllLoadErr("HAL_put")) {
				Log::error(LOG_TAG, "Error getting dlsym error! :(");				
			}

			return false;
		}

		mPropertiesCb = loadSymbol<properties_t>("HAL_properties");
		if (!mPropertiesCb) {
			if (!checkDllLoadErr("HAL_properties")) {
				Log::error(LOG_TAG, "Error getting dlsym error! :(");				
			}

			return false;
		}

		return true;
	}

private:
	void* mDllHndl;

	ShouseServerHAL::open_t mOpenCb;
	ShouseServerHAL::close_t mCloseCb;
	ShouseServerHAL::get_t mGetCb;
	ShouseServerHAL::put_t mPutCb;
	ShouseServerHAL::properties_t mPropertiesCb;

private:
	static constexpr const char* LOG_TAG = "ShouseServerHALdll";

};

// class ShouseHalFactory final {
// public:
// 	static std::unique_ptr<ShouseServerHAL> create(const char* path);

// };

// std::unique_ptr<ShouseServerHAL> ShouseHalManager::create(const char* path) {
// 	void* hndl = dlopen(path, RTLD_LAZY);
// 	if (!hndl) {
// 		return nullptr;
// 	}


// }
