#include <functional>

#include "shouse_res_client.h"

#include "Log.h"

using namespace OC;
using namespace std::placeholders;

OCStackResult ShouseResourceClient::get(const OC::QueryParamsMap& queryParametersMap) {
	OCStackResult ret;

	auto onGet = std::bind(&ShouseResourceClient::onGet, this, _1, _2, _3);

	ret = mResource->get(queryParametersMap, onGet);

	return ret;
}

OCStackResult ShouseResourceClient::put(const QueryParamsMap& queryParametersMap) {
	OCStackResult ret;

	auto onPut = std::bind(&ShouseResourceClient::onPut, this, _1, _2, _3);

	ret = mResource->put(mResourceRepr, queryParametersMap, onPut);

	return ret;
}

void ShouseResourceClient::onGet(const OC::HeaderOptions& opts, const OC::OCRepresentation& rep, const int eCode) {
	mResourceRepr = rep;
	mHal->onGet(opts, mResourceRepr, eCode);
}	

void ShouseResourceClient::onPut(const OC::HeaderOptions& opts, const OC::OCRepresentation& rep, const int eCode) {
	mResourceRepr = rep;
	mHal->onPut(opts, mResourceRepr, eCode);
}
