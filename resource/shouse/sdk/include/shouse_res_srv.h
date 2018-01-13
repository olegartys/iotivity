#ifndef SHOUSE_RES_SRV_H
#define SHOUSE_RES_SRV_H

#include <HAL/shouse_res_hal.h>

#include <shouse/baseresource.h>

class ShouseResourceServer : public BaseResourceServer {
public:
	ShouseResourceServer(const std::string& uri, const std::string& type, const std::string& iface, ShouseResHAL* hal) :
		BaseResourceServer(uri, type, iface), mHal(hal) { } 

	virtual OCStackResult createResource();

protected:
	virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OC::OCResourceRequest> request) override;

	virtual void sendResponse(const std::shared_ptr<OC::OCResourceResponse>& resp) const;

protected:
	ShouseResHAL* mHal;

private:
	static constexpr const char* LOG_TAG = "ShouseResourceServer";

};

#endif // SHOUSE_RES_SRV_H