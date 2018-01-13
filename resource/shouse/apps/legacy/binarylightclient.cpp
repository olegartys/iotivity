#include "binarylightclient.h"

void BinaryLightClient::onGet(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode) {
    Log::info(LOG_TAG, "{}: GET finished for light {}, eCode={}", __FUNCTION__, rep.getUri(), eCode);

    std::cout << "HEREGET: " << std::this_thread::get_id() << std::endl;

    if (!rep.getValue("state", mState)) {
        Log::error(LOG_TAG, "{}: Can't get light state for {}", __FUNCTION__, rep.getUri());
    }

    mResourceRepr.setValue("state", mState);

    Log::debug(LOG_TAG, "{}: Light state now = {}", __FUNCTION__, mState);
}

void BinaryLightClient::onPut(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode) {
    Log::info(LOG_TAG, "{}: PUT finished, ecode = {}", __FUNCTION__, eCode);

    if (!rep.getValue("state", mState)) {
        Log::error(LOG_TAG, "{}: Can't get light state for {}", __FUNCTION__, rep.getUri());
    }

    mResourceRepr.setValue("state", mState);

    Log::debug(LOG_TAG, "{}: Light state now = {}", __FUNCTION__, mState);
}
