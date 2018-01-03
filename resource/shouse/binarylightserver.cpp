#include "binarylightserver.h"

OCEntityHandlerResult BinaryLightServer::entityHandler(std::shared_ptr<OC::OCResourceRequest> request) {
    OCEntityHandlerResult ret = OC_EH_ERROR;
    std::string requestType = request->getRequestType();
    int requestFlag = request->getRequestHandlerFlag();

    // NOTE: setting handles is vital as iotivity doesn't know what
    // client routine to call when response is sent
    auto response = std::make_shared<OC::OCResourceResponse>();
    response->setRequestHandle(request->getRequestHandle());
    response->setResourceHandle(request->getResourceHandle());

    Log::info(LOG_TAG, "{}: New request with id = {}", __FUNCTION__, request->getMessageID());

    // Check the request destination
    if (request->getResourceUri() != mUri) {
        Log::error(LOG_TAG, "{}: request uri and real resource uri are not equal: {} {}",
                   request->getResourceUri(), mUri);
        // response->setErrorCode(404);
        response->setResponseResult(OC_EH_RESOURCE_NOT_FOUND);
        return OC_EH_RESOURCE_NOT_FOUND;
    }

    if (request) {
        // TODO: research Observe flag
        if (requestFlag & OC::RequestHandlerFlag::RequestFlag) {
            if (requestType == "GET") {
                Log::info(LOG_TAG, "{}: GET for the {}", __FUNCTION__, mUri);

                // response->setErrorCode(200);
                response->setResponseResult(OC_EH_OK);
                response->setResourceRepresentation(onGet());

                this->sendResponse(response);
                ret = OC_EH_OK;

            } else if (requestType == "PUT") {
                Log::info(LOG_TAG, "{}: PUT for the {}", __FUNCTION__, mUri);

                auto clientRepr = request->getResourceRepresentation();

                // Do all related operations to PUT request
                onPut(clientRepr, response);
                response->setResourceRepresentation(onGet());

                this->sendResponse(response);
                ret = OC_EH_OK;

            } else {
                Log::error(LOG_TAG, "{}: Unsupported request type: {}", __FUNCTION__, requestType);
                // response->setErrorCode(404);
                response->setResponseResult(OC_EH_METHOD_NOT_ALLOWED);

                this->sendResponse(response);
                ret = OC_EH_METHOD_NOT_ALLOWED;
            }
        }
    }

    return ret;
}

OC::OCRepresentation BinaryLightServer::onGet() {
    syncModelWithRepr();
    return mResourceRepr;
}

void BinaryLightServer::onPut(const OC::OCRepresentation& repr, std::shared_ptr<OC::OCResourceResponse> response) {
    if (!repr.getValue("state", mState)) {
        Log::error(LOG_TAG, "{}: Error doing PUT for {}", __FUNCTION__, mUri);
        // response->setErrorCode(200);
        response->setResponseResult(OC_EH_CONTENT);
        return;
    }

    syncModelWithRepr();
    // response->setErrorCode(200);
    response->setResponseResult(OC_EH_OK);

    Log::debug(LOG_TAG, "{}: Light {} state set to {}", __FUNCTION__, mUri, mState);
}
