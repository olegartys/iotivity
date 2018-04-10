#include <functional>

#include <shouse_res_client.h>

#include <shouse/Log.h>

using namespace OC;
using namespace std::placeholders;

OCStackResult ShouseResourceClient::get(const OC::QueryParamsMap& queryParametersMap,
	BaseResourceClient::onGetCb onGetHandler, bool async) {
	OCStackResult ret;

	auto onGet = std::bind(&ShouseResourceClient::onGet, this, onGetHandler,
		_1, _2, _3);

	ret = mOCResource->get(queryParametersMap, onGet);

	/* If we called synchronously, than wait for the 'onGet' to notfy that
	 * response from server has come.
	 */ 

	if (!async) {
		transactionWait();
	}

	return ret; 
}

OCStackResult ShouseResourceClient::put(const QueryParamsMap& queryParametersMap,
	BaseResourceClient::onGetCb onPutHandler, bool async) {
	OCStackResult ret;

	auto onPut = std::bind(&ShouseResourceClient::onPut, this, onPutHandler,
		_1, _2, _3);

	ret = mOCResource->put(mResource->repr(), queryParametersMap, onPut);

	/* If we called synchronously, than wait for the 'onGet' to notfy that
	 * response from server has come.
	 */ 

	if (!async) {
		transactionWait();
	}

	return ret;
}

void ShouseResourceClient::onGet(BaseResourceClient::onGetCb onGetHandler,
	const OC::HeaderOptions& opts,
	const OC::OCRepresentation& rep, const int eCode) {
	updateRepr(rep);

	if (onGetHandler)
		onGetHandler(opts, mPropertiesMap, eCode);

	/* Transaction has come, callback finished - we are ready to perform
	 * new transactions. Notify condition variable.
	 */ 

	transactionNotify();
}	

void ShouseResourceClient::onPut(BaseResourceClient::onPutCb onPutHandler,
	const OC::HeaderOptions& opts,
	const OC::OCRepresentation& rep, const int eCode) {
	updateRepr(rep);
	
	if (onPutHandler)
		onPutHandler(opts, mPropertiesMap, eCode);

	/* Transaction has come, callback finished - we are ready to perform
	 * new transactions. Notify condition variable.
	 */

	transactionNotify();
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

void ShouseResourceClient::transactionWait() const {
	mTransactionFinishedFlag = false;
	std::unique_lock<std::mutex> lock(mTransactionFinishedLock);
	mTransactionFinished.wait(lock,
		[this]() { return this->mTransactionFinishedFlag; });
}

void ShouseResourceClient::transactionNotify() const {
	mTransactionFinishedFlag = true;
	mTransactionFinished.notify_all();
}
