#ifndef SHOUSE_RES_SRV_H
#define SHOUSE_RES_SRV_H

#include <memory>
#include <atomic>
#include <thread>
#include <mutex>

#include <OCPlatform.h>
#include <OCApi.h>

#include <HAL/shouse_res_hal.h>
#include <dynamic_resource/dynamic_resource.h>

#include <shouse/base_resource_server.h>


class ShouseResourceServer : public BaseResourceServer<DynamicDataResource> {
public:
	ShouseResourceServer(const std::string& name,
		const std::string& uri, const std::string& type,
		const std::string& iface) : 
		BaseResourceServer(uri, type, iface),
		mName(std::move(name)), mObserverThreadStarted(false) {}

	~ShouseResourceServer();

	virtual bool createResource(ShouseServerHAL* hal);

	friend std::string to_string(const ShouseResourceServer& resourceServer);

protected:
	virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OC::OCResourceRequest> request) override;

	virtual void sendResponse(const std::shared_ptr<OC::OCResourceResponse>& resp) const;

	inline const ShouseResourceServer* getSelfPtr() { return this; }

protected:
	mutable std::recursive_mutex mResourceLock;
	inline void acquireResource() const { mResourceLock.lock(); }
	inline void releaseResource() const { mResourceLock.unlock(); }

protected:
	const std::string mName;
	ShouseServerHAL* mHal;
	int mId;

private:
	bool handleGET(const OC::QueryParamsMap& params);
	bool handlePUT(const OC::OCRepresentation& clientRepresentation, const OC::QueryParamsMap& params);
	void handleObserve(const OC::QueryParamsMap& params);

private:
	static constexpr const char* LOG_TAG = "ShouseResourceServer";

	OC::ObservationIds mObserverList;
	std::atomic_bool mObserverThreadStarted;
	std::thread mObserverThread;

	void startObserverThread(const OC::QueryParamsMap& params);
	void stopObserverThread();

	void notifyObservers(int halRet);

};

#endif // SHOUSE_RES_SRV_H
