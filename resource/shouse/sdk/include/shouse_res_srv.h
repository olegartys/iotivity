#ifndef SHOUSE_RES_SRV_H
#define SHOUSE_RES_SRV_H

#include <memory>

#include <HAL/shouse_res_hal.h>
#include <dynamic_resource/dynamic_resource.h>

#include <shouse/base_resource_server.h>


class ShouseResourceServer : public BaseResourceServer<DynamicDataResource> {
public:
	ShouseResourceServer(const std::string& uri, const std::string& type, const std::string& iface) : BaseResourceServer(uri, type, iface) {}

	virtual bool createResource(ShouseServerHAL* hal);

protected:
	virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OC::OCResourceRequest> request) override;

	virtual void sendResponse(const std::shared_ptr<OC::OCResourceResponse>& resp) const;

	inline const ShouseResourceServer* getSelfPtr() { return this; }

protected:
	ShouseServerHAL* mHal;

private:
	bool handleGET(const OC::QueryParamsMap& params);
	bool handlePUT(const OC::OCRepresentation& clientRepresentation, const OC::QueryParamsMap& params);

private:
	static constexpr const char* LOG_TAG = "ShouseResourceServer";

};

#endif // SHOUSE_RES_SRV_H