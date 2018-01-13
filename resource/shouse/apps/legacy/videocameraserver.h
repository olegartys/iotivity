#ifndef VIDEOCAMERASERVER_H
#define VIDEOCAMERASERVER_H

#include "videocameraresource.h"
#include "Log.h"

namespace PH = std::placeholders;

class VideoCameraServer : public BaseResourceServer,
        public VideoCameraTypeInterface, public VideoCameraModel {
public:
    VideoCameraServer(const std::string& uri) :
        BaseResourceServer(uri) {

    }

    OCStackResult createResource() {
        OC::EntityHandler cb = std::bind(&VideoCameraServer::entityHandler, this, PH::_1);
        return OC::OCPlatform::registerResource(mResourceHandle, mUri, mType, mIface, cb,
                                            (uint8_t)(OC_DISCOVERABLE | OC_OBSERVABLE));
    }

private:
    static constexpr char* LOG_TAG = "VideoCameraServer";

    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OC::OCResourceRequest> request) override;

    OC::OCRepresentation onGet();

    void sendResponse(const std::shared_ptr<OC::OCResourceResponse>& resp) const {
        Log::info(LOG_TAG, "{}: Sending response...", __FUNCTION__);
        if (OC::OCPlatform::sendResponse(resp) != OC_STACK_OK) {
            Log::error(LOG_TAG, "{}: failed!", __FUNCTION__);
        }
    }
};

#endif // VIDEOCAMERASERVER_H
