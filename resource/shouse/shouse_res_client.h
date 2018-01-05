#ifndef SHOUSE_RES_CLIENT_H
#define SHOUSE_RES_CLIENT_H

#include "baseresource.h"
#include "shouse_res_hal.h"

class ShouseResourceClient : public BaseResourceClient {
public:
	ShouseResourceClient(const std::string& uri, const std::string& type, const std::string& iface, ShouseClientHAL* hal) :
		BaseResourceClient(uri, type, iface), mHal(hal)
	{ }

	virtual void setResource(const std::shared_ptr<OC::OCResource>& resource) override {
		mResource = resource;
	}

	virtual OCStackResult get(const OC::QueryParamsMap& queryParametersMap) override;

    virtual OCStackResult put(const OC::QueryParamsMap& queryParametersMap) override;

protected:
	std::shared_ptr<OC::OCResource> mResource;
	ShouseClientHAL* mHal;

	virtual void onGet(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode);

    virtual void onPut(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode);

private:
	static constexpr const char* LOG_TAG = "ShouseResourceClient"; 

};

#endif // SHOUSE_RES_CLIENT_H