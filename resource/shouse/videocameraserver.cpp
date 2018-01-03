#include "videocameraserver.h"
#include "Log.h"

OCEntityHandlerResult VideoCameraServer::entityHandler(std::shared_ptr<OC::OCResourceRequest> request) {
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

OC::OCRepresentation VideoCameraServer::onGet() {
    mResourceRepr.setValue("data", nextFrameBase64());
    return mResourceRepr;
}

