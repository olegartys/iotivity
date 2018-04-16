#include <iomanip>
#include <functional>

#include <shouse_res_srv.h>

#include <shouse/Log.h>

using namespace OC;
using namespace std::placeholders; 

bool ShouseResourceServer::createResource(ShouseServerHAL* hal) {
    if (!hal) {
        Log::error(LOG_TAG, "HAL pointer is invalid");
        return false;
    }

    mHal = hal;

    // Try to obtain id of device from HAL
    mId = mHal->open();
    if (mId < 0) {
        Log::error(LOG_TAG, "Error openning HAL device");
        return false;
    }

    // Initialize underlying Resource with resource properties
    mResource->init(mHal->properties());

    // Register callback for resource requests
    EntityHandler cb = std::bind(&ShouseResourceServer::entityHandler, this, _1);

    // Make the resource visible over the network
    auto _uri = mResource->uri();

    auto regResult = OCPlatform::registerResource(mResourceHandle, _uri, type(),
        iface(), cb, (uint8_t)(OC_DISCOVERABLE | OC_OBSERVABLE));

    return regResult == OC_STACK_OK;
}

OCEntityHandlerResult ShouseResourceServer::entityHandler(std::shared_ptr<OCResourceRequest> request) {
    OCEntityHandlerResult ret = OC_EH_ERROR;

    std::string requestType = request->getRequestType();
    int requestFlag = request->getRequestHandlerFlag();

    auto response = std::make_shared<OCResourceResponse>();

    // NOTE: setting handles is vital as iotivity doesn't know what
    // client routine to call when response is sent
    response->setRequestHandle(request->getRequestHandle());
    response->setResourceHandle(request->getResourceHandle());

    Log::info(LOG_TAG, "{}: New request with id = {}", __FUNCTION__, request->getMessageID());

    // Check the request destination
    if (request->getResourceUri() != mResource->uri()) {
        Log::error(LOG_TAG, "{}: request uri and real resource uri are not equal: {} {}",
                   request->getResourceUri(), mResource->uri());

        response->setResponseResult(OC_EH_RESOURCE_NOT_FOUND);

        return OC_EH_RESOURCE_NOT_FOUND;
    }

    if (request) {
    	auto clientRepresentation = request->getResourceRepresentation();
        auto queryParams = request->getQueryParameters();

        /* Client has made request - so here we are.
         * Let HAL do all the 'get' and 'put' job.
         */

        if (requestFlag & RequestHandlerFlag::RequestFlag) {
            if (requestType == "GET") {
                Log::info(LOG_TAG, "{}: GET for the {}", __FUNCTION__, mResource->uri());
                
                ret = handleGET(queryParams) ? OC_EH_OK : OC_EH_ERROR;

            } else if (requestType == "PUT") {
                Log::info(LOG_TAG, "{}: PUT for the {}", __FUNCTION__, mResource->uri());
                
                ret = handlePUT(clientRepresentation, queryParams) ? OC_EH_OK : OC_EH_ERROR;

            } else {
                Log::error(LOG_TAG, "{}: Unsupported request type: {}", __FUNCTION__, requestType);

                ret = OC_EH_METHOD_NOT_ALLOWED;
            }

            response->setResponseResult(ret);
            response->setResourceRepresentation(mResource->repr());

            sendResponse(response);
        }

        /* Client requested to observe the resource.
         *
         */

        if (requestFlag & RequestHandlerFlag::ObserverFlag) {

            /* Check what is the intention of observe request - whether
             * to subscribe to the resource or to unsubscribe from it.
             */

            ObservationInfo observationInfo = request->getObservationInfo();
            if (observationInfo.action == ObserveAction::ObserveRegister) {
                if (!mObserverThreadStarted) {
                    startObserverThread(queryParams);
                }

                mObserverList.push_back(observationInfo.obsId);

            } else if (observationInfo.action == ObserveAction::ObserveUnregister) {
                mObserverList.erase(
                    std::remove(mObserverList.begin(), mObserverList.end(),
                        observationInfo.obsId),
                    mObserverList.end()
                );

                if (mObserverThreadStarted && mObserverList.empty()) {
                    stopObserverThread();
                }
            } else {

                Log::error(LOG_TAG, "Unsupperted observer action");

 
                ret = OC_EH_METHOD_NOT_ALLOWED;
            }
        }

        // TODO: Do we need to send response from here?
    }

    return ret;
}

void ShouseResourceServer::startObserverThread(const QueryParamsMap& params) {
    mObserverThreadStarted = true;
    mObserverThread = std::thread([this, params]
        { handleObserve(params); }
    );
    mObserverThread.detach();
}

void ShouseResourceServer::stopObserverThread() {
    Log::info(LOG_TAG, "Observers list is empty, stopping thread");
    mObserverThreadStarted = false;
}

void ShouseResourceServer::notifyObservers(int halRet) {
    auto response = std::make_shared<OCResourceResponse>();
    OCEntityHandlerResult ret = OC_EH_OK;

    acquireResource();

    bool getResult = handleGET(QueryParamsMap());
    if (!getResult) {
        Log::error(LOG_TAG, "{}: Error getting resource data from HAL",
            __FUNCTION__);
        ret = OC_EH_ERROR;
    }

    response->setResponseResult(ret);
    response->setResourceRepresentation(mResource->repr());

    releaseResource();

    auto result = OCPlatform::notifyListOfObservers(hndl(), mObserverList,
        response);
    if (result == OC_STACK_NO_OBSERVERS) {
        Log::warn(LOG_TAG, "No more observers exists");
        stopObserverThread();
    }
}

void ShouseResourceServer::handleObserve(const QueryParamsMap& params) {
    std::function<void(int)> f = [this](int ret) { notifyObservers(ret); };
    mHal->observe(mId, params, mObserverThreadStarted, f);
}

bool ShouseResourceServer::handleGET(const QueryParamsMap& params) {
    ShouseHALResult halRet;

    acquireResource();

    for (const auto& prop: mHal->properties()) {
        // Request property from HAL
        std::string propValue;
        halRet = mHal->get(mId, prop.mName, propValue, params);

        if (halRet != ShouseHALResult::OK) {
            Log::error(LOG_TAG, "HAL failed to get {}", prop.mName);
            releaseResource();
            return false;
        }

        // Update the resource data with the new value
        mResource->setProp(prop, propValue);
    }

    releaseResource();

    return true;
}

bool ShouseResourceServer::handlePUT(const OCRepresentation& clientRepresentation,
    const QueryParamsMap& params) {
    ShouseHALResult halRet = ShouseHALResult::ERR;

    acquireResource();

    for (const auto& prop: mHal->properties()) {
        // Request property from HAL
        std::string newValue;

        bool ret = clientRepresentation.getValue(prop.mName, newValue);
        if (!ret) {
            Log::error(LOG_TAG, "Error getting value {} from OCRepr", prop.mName);
            break;
        }

        // Parse received value into ResourceProperty object

        ResourceProperty parsedProp;
        ret = parsedProp.fromJson(newValue);
        if (!ret) {
            Log::error(LOG_TAG, "Error parsing property {} value {} to "
                "ResourceProperty", prop.mName, newValue);
            break;
        }

        // If the key in OCRepresentation for the property is not equal
        // to the name in its JSON representation - something wrong has 
        // happened.

        if (parsedProp.mName != prop.mName) {
            Log::error(LOG_TAG, "parsedProp.mName != prop.mName O_o: "
                "{} != {}", parsedProp.mName, prop.mName);
            break;
        }

        // Send update request to the HAL

        halRet = mHal->put(mId, parsedProp.mName, parsedProp.mValue, params);
        if (halRet != ShouseHALResult::OK) {
            Log::error(LOG_TAG, "HAL failed to set {}", prop.mName);
            releaseResource();
            return false;
        }

        // Update the representation with the new value
        mResource->setProp(prop, newValue);
    }

    releaseResource();

    return true;
}

static int count = 0;

void ShouseResourceServer::sendResponse(std::shared_ptr<OCResourceResponse> resp) const {
    Log::info(LOG_TAG, "{}: Sending response...", __FUNCTION__);
    if (type() == "camera") {
        Log::error(LOG_TAG, "{}: Sending response {}...", count++, __FUNCTION__);
    }
    if (OCPlatform::sendResponse(resp) != OC_STACK_OK) {
        Log::error(LOG_TAG, "{}: failed!", __FUNCTION__);
    }
}

ShouseResourceServer::~ShouseResourceServer() {
    // Close HAL device

    if (mHal) {
        mHal->close(mId);
    }
}

std::string to_string(const ShouseResourceServer& resourceServer) {
    std::stringstream ss;

    ss << "Resource " << resourceServer.mName << "\n";
    ss << std::setw(10) << "uri:\t" << resourceServer.mResource->uri() << '\n'
       << std::setw(10) << "type:\t" << resourceServer.mResource->type() << '\n'
       << std::setw(10) << "iface:\t" << resourceServer.mResource->iface() << '\n';

    ss << std::setw(10) << "properties: \n";

    for (const auto& defaultProp: resourceServer.mHal->properties()) {
        auto propName = defaultProp.mName;

        // Get current property value from Resource

        ResourceProperty prop;
        if (resourceServer.mResource->getProp(prop, propName)) {
            ss << std::setw(20) << propName << "\n" 
               << std::setw(20) << to_string(prop) << "\n";
        }
    }

    return ss.str();
}
