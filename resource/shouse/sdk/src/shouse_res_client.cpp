#include <functional>

#include <shouse_res_client.h>

#include <shouse/Log.h>

using namespace OC;
using namespace std::placeholders;

OCStackResult ShouseResourceClient::get(const OC::QueryParamsMap& queryParametersMap) {
	OCStackResult ret;

	auto onGet = std::bind(&ShouseResourceClient::onGet, this, _1, _2, _3);

	ret = mOCResource->get(queryParametersMap, onGet);

	return ret; 
}

OCStackResult ShouseResourceClient::put(const QueryParamsMap& queryParametersMap) {
	OCStackResult ret;

	auto onPut = std::bind(&ShouseResourceClient::onPut, this, _1, _2, _3);

	auto values = mResource->repr().getValues();
	for (auto& val: values) {
		Log::error(LOG_TAG, "Sending val: {}", val.first);
	}

	ret = mOCResource->put(mResource->repr(), queryParametersMap, onPut);

	return ret;
}

void ShouseResourceClient::onGet(const OC::HeaderOptions& opts, const OC::OCRepresentation& rep, const int eCode) {
	updateRepr(rep);
	mHal->onGet(opts, mPropertiesMap, eCode);
}	

void ShouseResourceClient::onPut(const OC::HeaderOptions& opts, const OC::OCRepresentation& rep, const int eCode) {
	updateRepr(rep);
	mHal->onPut(opts, mPropertiesMap, eCode);
}

void ShouseResourceClient::updateRepr(const OC::OCRepresentation& rep) {
	mResource->setRepr(rep);

	auto values = mResource->repr().getValues();

	for (const auto& val: values) {
		auto propName = val.first;
		ResourceProperty prop;

		if (!mResource->getProp(prop, propName)) {
			Log::error(LOG_TAG, "Error getting property {}", propName);
		} else {
			mPropertiesMap[propName] = prop;
		}
	}
}
