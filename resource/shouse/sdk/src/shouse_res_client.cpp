#include <functional>

#include <shouse_res_client.h>

#include <shouse/Log.h>

using namespace OC;
using namespace std::placeholders;

OCStackResult ShouseResourceClient::get(const OC::QueryParamsMap& queryParametersMap,
	BaseResourceClient::onGetCb onGetHandler) {
	OCStackResult ret;

	auto onGet = std::bind(&ShouseResourceClient::onGet, this, onGetHandler,
		_1, _2, _3);

	ret = mOCResource->get(queryParametersMap, onGet);

	return ret; 
}

OCStackResult ShouseResourceClient::put(const QueryParamsMap& queryParametersMap,
	BaseResourceClient::onGetCb onPutHandler) {
	OCStackResult ret;

	auto onPut = std::bind(&ShouseResourceClient::onPut, this, onPutHandler,
		_1, _2, _3);

	ret = mOCResource->put(mResource->repr(), queryParametersMap, onPut);

	return ret;
}

void ShouseResourceClient::onGet(BaseResourceClient::onGetCb onGetHandler,
	const OC::HeaderOptions& opts,
	const OC::OCRepresentation& rep, const int eCode) {
	updateRepr(rep);
	onGetHandler(opts, mPropertiesMap, eCode);
}	

void ShouseResourceClient::onPut(BaseResourceClient::onPutCb onPutHandler,
	const OC::HeaderOptions& opts,
	const OC::OCRepresentation& rep, const int eCode) {
	updateRepr(rep);
	onPutHandler(opts, mPropertiesMap, eCode);
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
