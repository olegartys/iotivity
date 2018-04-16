#include <functional>
#include <sstream>
#include <iomanip>

#include <shouse_res_client.h>

#include <shouse/Log.h>

using namespace OC;
using namespace std::placeholders;

OCStackResult ShouseResourceClient::get(const OC::QueryParamsMap& queryParametersMap,
	BaseResourceClient::onGetCb onGetHandler) {
	OCStackResult ret;

	auto onGet = std::bind(&ShouseResourceClient::onGet, this, onGetHandler,
		_1, _2, _3);

	acquireResource();
	ret = mOCResource->get(queryParametersMap, onGet);
	releaseResource();

	/* If we called synchronously, than wait for the 'onGet' to notfy that
	 * response from server has come.
	 */ 

	if (!mIsAsync) {
		transactionWait();
	}

	return ret; 
}

OCStackResult ShouseResourceClient::put(const QueryParamsMap& queryParametersMap,
	BaseResourceClient::onGetCb onPutHandler) {
	OCStackResult ret;

	auto onPut = std::bind(&ShouseResourceClient::onPut, this, onPutHandler,
		_1, _2, _3);

	acquireResource();
	ret = mOCResource->put(mResource->repr(), queryParametersMap, onPut);
	releaseResource();

	/* If we called synchronously, than wait for the 'onGet' to notfy that
	 * response from server has come.
	 */ 

	if (!mIsAsync) {
		transactionWait();
	}

	return ret;
}

OCStackResult ShouseResourceClient::startObserve(
	const OC::QueryParamsMap& queryParametersMap,
	onObserveCb onObserveHandler) {
	OCStackResult ret;

	auto onObserve = std::bind(&ShouseResourceClient::onObserve, this,
		onObserveHandler,
		_1, _2, _3, _4);

	isObserveStarted = true;

	ret = mOCResource->observe(ObserveType::Observe, queryParametersMap,
		onObserve);

	/* We do not care about whether the resource is async or not.
	 * Observation is always asynchronous operation by its nature
	 * and we can not synchronize with PUT/GET in any way beside
	 * acquiring resource instance.
	 */

	return ret;
}

OCStackResult ShouseResourceClient::stopObserve() {
	OCStackResult ret = OC_STACK_OK;

	if (isObserveStarted) {
		ret = mOCResource->cancelObserve();
		isObserveStarted = false;
	}

	return ret;
}

bool ShouseResourceClient::setProp(const std::string& name,
	const std::string& value) {

	acquireResource();

	auto curProp = mPropertiesMap[name];

	// Update resource representation and internal map of props

	auto ret = mResource->setProp(curProp, value);
	if (ret) {
	    mPropertiesMap[name] = curProp;    		
	}

	releaseResource();

	return ret;
}

void ShouseResourceClient::onGet(BaseResourceClient::onGetCb onGetHandler,
	const OC::HeaderOptions& opts,
	const OC::OCRepresentation& rep, const int eCode) {
	acquireResource();

	updateRepr(rep);

	if (onGetHandler)
		onGetHandler(opts, mPropertiesMap, eCode);

	releaseResource();

	/* Transaction has come, callback finished - we are ready to perform
	 * new transactions. Notify condition variable.
	 */ 

	transactionNotify();
}	

void ShouseResourceClient::onPut(BaseResourceClient::onPutCb onPutHandler,
	const OC::HeaderOptions& opts,
	const OC::OCRepresentation& rep, const int eCode) {
	acquireResource();

	updateRepr(rep);
	
	if (onPutHandler)
		onPutHandler(opts, mPropertiesMap, eCode);

	releaseResource();

	/* Transaction has come, callback finished - we are ready to perform
	 * new transactions. Notify condition variable.
	 */

	transactionNotify();
}

void ShouseResourceClient::onObserve(
	BaseResourceClient::onObserveCb onObserveHandler,
	const HeaderOptions& opts,
	const OCRepresentation& rep, const int eCode,
	const int& sequenceNumber) {
	if (eCode == OC_STACK_OK) {
		if (sequenceNumber == OC_OBSERVE_REGISTER) {
			Log::info(LOG_TAG, "Observe registration action is successful");
		}

		acquireResource();

		updateRepr(rep);

		if (onObserveHandler)
			onObserveHandler(opts, mPropertiesMap, eCode);

		releaseResource();

	} else {
		Log::warn(LOG_TAG, "Error performing 'observe' transaction:{}",
			eCode);
	}
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
	std::unique_lock<std::mutex> lock(mTransactionLock);
	mTransactionFinished.wait(lock,
		[this]() { return this->mTransactionFinishedFlag; });
}

void ShouseResourceClient::transactionNotify() const {
	mTransactionFinishedFlag = true;
	mTransactionFinished.notify_all();
}

std::string to_string(const ShouseResourceClient& resourceClient) {
	std::stringstream ss;

    ss << "Resource " << resourceClient.mName << "\n";
    ss << std::setw(10) << "uri:\t" << resourceClient.mResource->uri() << '\n'
       << std::setw(10) << "type:\t" << resourceClient.mResource->type() << '\n'
       << std::setw(10) << "iface:\t" << resourceClient.mResource->iface() << '\n';

    ss << std::setw(10) << "properties: \n";

    resourceClient.acquireResource();

    for (const auto& prop: resourceClient.mPropertiesMap) {
		ss << std::setw(20) << prop.first << "\n" 
		   << std::setw(20) << to_string(prop.second) << "\n";
    }

    resourceClient.releaseResource();

    return ss.str();
}
