#include <functional>

#include <shouse_res_srv.h>

#include <shouse/Log.h>

using namespace std::placeholders;

OCEntityHandlerResult ShouseResourceServer::entityHandler(std::shared_ptr<OC::OCResourceRequest> request) {
    OCEntityHandlerResult ret = OC_EH_ERROR;

    std::string requestType = request->getRequestType();
    int requestFlag = request->getRequestHandlerFlag();

    auto response = std::make_shared<OC::OCResourceResponse>();

    // NOTE: setting handles is vital as iotivity doesn't know what
    // client routine to call when response is sent
    response->setRequestHandle(request->getRequestHandle());
    response->setResourceHandle(request->getResourceHandle());

    Log::info(LOG_TAG, "{}: New request with id = {}", __FUNCTION__, request->getMessageID());

    // Check the request destination
    if (request->getResourceUri() != mUri) {
        Log::error(LOG_TAG, "{}: request uri and real resource uri are not equal: {} {}",
                   request->getResourceUri(), mUri);

        response->setResponseResult(OC_EH_RESOURCE_NOT_FOUND);

        return OC_EH_RESOURCE_NOT_FOUND;
    }

    if (request) {
    	auto clientRepresentation = request->getResourceRepresentation();

        // TODO: research Observe flag
        if (requestFlag & OC::RequestHandlerFlag::RequestFlag) {
            if (requestType == "GET") {
                Log::info(LOG_TAG, "{}: GET for the {}", __FUNCTION__, mUri);

                mResourceRepr = clientRepresentation;

                auto halRet = mHal->onGet(repr(), request->getQueryParameters());

                Log::info(LOG_TAG, "HAL returned: {}", halRet);

                ret = halRet == 0 ? OC_EH_OK : OC_EH_ERROR;

            } else if (requestType == "PUT") {
                Log::info(LOG_TAG, "{}: PUT for the {}", __FUNCTION__, mUri);

				mResourceRepr = clientRepresentation;

                auto halRet = mHal->onPut(repr(), request->getQueryParameters());

                Log::info(LOG_TAG, "HAL returned: {}", halRet);

                ret = halRet == 0 ? OC_EH_OK : OC_EH_ERROR;

            } else {
                Log::error(LOG_TAG, "{}: Unsupported request type: {}", __FUNCTION__, requestType);

                ret = OC_EH_METHOD_NOT_ALLOWED;
            }
        }

        response->setResponseResult(ret);
        response->setResourceRepresentation(mResourceRepr);

        this->sendResponse(response);
    }

    return ret;
}

OCStackResult ShouseResourceServer::createResource() {
	OC::EntityHandler cb = std::bind(&ShouseResourceServer::entityHandler, this, _1);

    return OC::OCPlatform::registerResource(mResourceHandle, mUri, mType, mIface, cb, (uint8_t)(OC_DISCOVERABLE | OC_OBSERVABLE));
}

void ShouseResourceServer::sendResponse(const std::shared_ptr<OC::OCResourceResponse>& resp) const {
    Log::info(LOG_TAG, "{}: Sending response...", __FUNCTION__);
    if (OC::OCPlatform::sendResponse(resp) != OC_STACK_OK) {
        Log::error(LOG_TAG, "{}: failed!", __FUNCTION__);
    }
}
