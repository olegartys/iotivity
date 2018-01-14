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

	ret = mOCResource->put(mResource->repr(), queryParametersMap, onPut);

	return ret;
}

void ShouseResourceClient::onGet(const OC::HeaderOptions& opts, const OC::OCRepresentation& rep, const int eCode) {
	mResource->setRepr(rep);
	mHal->onGet(opts, mResource->repr(), eCode);
}	

void ShouseResourceClient::onPut(const OC::HeaderOptions& opts, const OC::OCRepresentation& rep, const int eCode) {
	mResource->setRepr(rep);
	mHal->onPut(opts, mResource->repr(), eCode);
}
