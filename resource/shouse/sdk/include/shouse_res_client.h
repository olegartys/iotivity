#ifndef SHOUSE_RES_CLIENT_H
#define SHOUSE_RES_CLIENT_H

#include <HAL/shouse_res_hal.h>

#include <shouse/base_resource_client.h>

#include <dynamic_resource/dynamic_resource.h>

class ShouseResourceClient : public BaseResourceClient<DynamicDataResource> {
public:
	ShouseResourceClient(const std::string& uri, const std::string& type, const std::string& iface, ShouseClientHAL* hal) :
		BaseResourceClient(uri, type, iface), mHal(hal) {}

	virtual OCStackResult get(const OC::QueryParamsMap& queryParametersMap) override;

    virtual OCStackResult put(const OC::QueryParamsMap& queryParametersMap) override;

protected:
	ShouseClientHAL* mHal;

	virtual void onGet(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode);

    virtual void onPut(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode);

private:
	static constexpr const char* LOG_TAG = "ShouseResourceClient"; 

};

#endif // SHOUSE_RES_CLIENT_H
