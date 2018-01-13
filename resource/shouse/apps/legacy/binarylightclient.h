#ifndef BINARYLIGHTCLIENT_H
#define BINARYLIGHTCLIENT_H

#include "lightresource.h"

class BinaryLightClient : public BaseResourceClient,
        public BinaryLightTypeInterface, public BinaryLightModel {
public:
    BinaryLightClient() :
        BinaryLightModel(false) {
        mResourceRepr.setValue("state", false);
    }

    void enable() { mState = true; mResourceRepr.setValue("state", mState); }
    void disable() { mState = false; mResourceRepr.setValue("state", mState); }

    virtual void onGet(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode);

    virtual void onPut(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode);

private:
    static constexpr char* LOG_TAG = "BinaryLightClient";

};

#endif // BINARYLIGHTCLIENT_H
