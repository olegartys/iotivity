#include <opencv2/opencv.hpp>

#include "videocameraclient.h"

#include <thread>
#include <condition_variable>

#include "Log.h"

extern std::mutex gMutex;
extern std::condition_variable gCondVar;

void VideoCameraClient::onGet(const OC::HeaderOptions&, const OC::OCRepresentation& rep, const int eCode) {
    Log::info(LOG_TAG, "{}: new frame from {}, ecode={}", __FUNCTION__, rep.getUri(), eCode);

    if (!rep.getValue("data", mCurFrameBase64)) {
        Log::error(LOG_TAG, "{}: can't read data frame payload!", __FUNCTION__);
    }

   // mResourceRepr.setValue("data", mCurFrameBase64);

    // Decode data
    size_t outSize = 0;
    auto decodeframe = this->curFramePtr(mCurFrameBase64, outSize);

    // NOTE: Zero-copy
    cv::_InputArray arr(decodeframe.get(), (int)outSize);

    cv::Mat imgMat = cv::imdecode(arr, CV_LOAD_IMAGE_COLOR);
    cv::imshow("kek", imgMat);
    cv::waitKey(20);

    Log::debug(LOG_TAG, "len={} width={} height={}", arr.total(), imgMat.cols, imgMat.rows);
    cv::imwrite("/tmp/t.jpg", imgMat);

    // Frame processing end, request new frame
    gCondVar.notify_all();
}
