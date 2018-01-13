#ifndef VIDEOCAMERACLIENT_H
#define VIDEOCAMERACLIENT_H

#include "videocameraresource.h"

class VideoCameraClient : public BaseResourceClient,
        public VideoCameraTypeInterface, public VideoCameraModel {
public:
    VideoCameraClient() {     cv::namedWindow("kek"); }

    virtual void onGet(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode) override;

private:
    static const constexpr char* LOG_TAG = "VideoCameraClient";

};

#endif // VIDEOCAMERACLIENT_H
