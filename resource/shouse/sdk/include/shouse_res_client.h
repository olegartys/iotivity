#ifndef SHOUSE_RES_CLIENT_H
#define SHOUSE_RES_CLIENT_H

#include <functional>
#include <mutex>
#include <condition_variable>

#include <HAL/shouse_res_hal.h>

#include <shouse/base_resource_client.h>

#include <dynamic_resource/dynamic_resource.h>

class ShouseResourceClient : public BaseResourceClient<DynamicDataResource> {
public:
	ShouseResourceClient(const std::string& uri, const std::string& type,
		const std::string& iface) :
		BaseResourceClient(uri, type, iface) {}

	virtual OCStackResult get(const OC::QueryParamsMap& queryParametersMap,
		onGetCb onGet, bool async) override;

    virtual OCStackResult put(const OC::QueryParamsMap& queryParametersMap,
    	onPutCb onPut, bool async) override;

    OCStackResult get(onGetCb onGet = nullptr, bool async = false) {
    	OC::QueryParamsMap queryParametersMap;
    	return get(queryParametersMap, onGet, async);
    }

    OCStackResult put(onPutCb onPut = nullptr, bool async = false) {
    	OC::QueryParamsMap queryParametersMap;
    	return put(queryParametersMap, onPut, async);
    }

    bool setProp(const std::string& name, const std::string& value) {
    	auto curProp = mPropertiesMap[name];

    	// Update resource representation and internal map of props

    	auto ret = mResource->setProp(curProp, value);
    	if (ret) {
		    mPropertiesMap[name] = curProp;    		
    	}

    	return ret;
    }

    const std::map<std::string, ResourceProperty>& properties() const {
    	return mPropertiesMap;
    }

protected:
	virtual void onGet(BaseResourceClient::onGetCb onGetHandler,
		const OC::HeaderOptions& opts,
		const OC::OCRepresentation& rep, const int eCode);

    virtual void onPut(BaseResourceClient::onPutCb onPutHandler,
		const OC::HeaderOptions& opts,
		const OC::OCRepresentation& rep, const int eCode);

private:
	static constexpr const char* LOG_TAG = "ShouseResourceClient"; 

	void updateRepr(const OC::OCRepresentation& rep);

	std::map<std::string, ResourceProperty> mPropertiesMap;

private:
	mutable std::condition_variable mTransactionFinished;
	mutable std::mutex mTransactionFinishedLock;
	mutable bool mTransactionFinishedFlag;

	void transactionWait() const;
	void transactionNotify() const;

};

#endif // SHOUSE_RES_CLIENT_H
