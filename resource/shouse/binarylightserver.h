#ifndef BINARYLIGHTSERVER_H
#define BINARYLIGHTSERVER_H

#include "Log.h"
#include "lightresource.h"

class BinaryLightServer : public BaseResourceServer,
        public BinaryLightTypeInterface, public BinaryLightModel {
public:
    BinaryLightServer(const std::string& uri) :
        BaseResourceServer(uri), BinaryLightModel(false) {
    }

    OCStackResult createResource() {
        OC::EntityHandler cb = std::bind(&BinaryLightServer::entityHandler, this, PH::_1);
        return OC::OCPlatform::registerResource(mResourceHandle, mUri, mType, mIface, cb,
                                            (uint8_t)(OC_DISCOVERABLE | OC_OBSERVABLE));
    }

    void enable() { mState = true; }
    void disable() { mState = false; }

private:
    virtual OCEntityHandlerResult entityHandler(std::shared_ptr<OC::OCResourceRequest> request) override;

    OC::OCRepresentation onGet();

    void onPut(const OC::OCRepresentation& repr, std::shared_ptr<OC::OCResourceResponse> response);

private:
    static constexpr char* LOG_TAG = "BinaryLightServer";

    void syncModelWithRepr() {
        mResourceRepr.setValue("state", mState);
    }

    void sendResponse(const std::shared_ptr<OC::OCResourceResponse>& resp) const {
        Log::info(LOG_TAG, "{}: Sending response...", __FUNCTION__);
        if (OC::OCPlatform::sendResponse(resp) != OC_STACK_OK) {
            Log::error(LOG_TAG, "{}: failed!", __FUNCTION__);
        }
    }

};

#endif // BINARYLIGHTSERVER_H
