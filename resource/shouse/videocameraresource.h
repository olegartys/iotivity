#ifndef VIDEOCAMERARESOURCE_H
#define VIDEOCAMERARESOURCE_H

#include <opencv2/opencv.hpp>

#include <opencv2/videoio.hpp>

#include "Log.h"
#include "baseresource_old.h"
#include "base64.h"

class VideoCameraTypeInterface : public BaseResourceTypeInterface {
protected:
    VideoCameraTypeInterface() :
        BaseResourceTypeInterface("type.camera_30fps", "iface.camera") {}

};

class VideoCameraModel {
public:
    VideoCameraModel() { }

    bool init() {
        if (!mWebCamCap.open(0)) {
            Log::error(LOG_TAG, "Error openning webcam 0");
            return false;
        } else {
            mFrameWidth = mWebCamCap.get(cv::CAP_PROP_FRAME_WIDTH);
            mFrameHeight = mWebCamCap.get(cv::CAP_PROP_FRAME_HEIGHT);
            mFps = mWebCamCap.get(cv::CAP_PROP_FPS);
            mFormat = mWebCamCap.get(cv::CAP_PROP_FORMAT);

            mMaxFrameSize = mFrameWidth * mFrameHeight;

            Log::debug(LOG_TAG, "Video camera intialized: w={} h={} framesize = {}, fps={},"
                                "format={}",
                       mFrameWidth, mFrameHeight, mMaxFrameSize, mFps, mFormat);
        }

        return true;
    }

    std::string nextFrameBase64() {
        mWebCamCap >> mCurFrame;

        Log::debug(LOG_TAG, "Format of read frame: {}", mCurFrame.type());


        if (!mCurFrame.empty()) {
            Log::debug(LOG_TAG, "Read frame, w={} h={} total={} elemsize={}",
                       mCurFrame.cols, mCurFrame.rows,
                       mCurFrame.total(),
                       mCurFrame.elemSize());
            mCurFramePtr = mCurFrame.ptr();

            cv::imshow("srv", mCurFrame);
            cv::waitKey(20);

            std::vector<uchar> compressedFrame;
            cv::imencode(".jpg", mCurFrame, compressedFrame);
            Log::debug(LOG_TAG, "Compressed frame size: {}", compressedFrame.size());

            // Get the size of output buffer
            size_t outBufFakeSize = 0;
            size_t outBufRealSize = 0;
            int ret = b64Encode(compressedFrame.data(), compressedFrame.size()/*mCurFrame.total() * mCurFrame.elemSize()*/,
                                (char*)mCurFramePtr, (size_t)outBufFakeSize, &outBufRealSize);
            if (ret != B64_OUTPUT_BUFFER_TOO_SMALL) {
                Log::error(LOG_TAG, "Geting output buffer size failed: {}", ret);
                return "";
            }

            Log::debug(LOG_TAG, "New output buffer size: {}", outBufRealSize);

            // Encode buffer
            std::shared_ptr<char> outBuf(new char[outBufRealSize*3]);
            ret = b64Encode(compressedFrame.data(), compressedFrame.size()/*mCurFrame.total() * mCurFrame.elemSize()*/,
                            outBuf.get(), (size_t)outBufRealSize*3, &outBufRealSize);
            if (ret != B64_OK) {
                Log::error(LOG_TAG, "Encode64 failed: {}", ret);
                return "";
            }

            mCurFrameBase64.assign(outBuf.get());

            return mCurFrameBase64;
        }

        return "";
    }

    void close() {
        mWebCamCap.release();
    }

    const std::shared_ptr<uint8_t> curFramePtr(const std::string& base64str, size_t& outSize) {
        // Get the size of output buffer
        size_t outBufFakeSize = 0;
        size_t outBufRealSize = 0;
        int ret = b64Decode(base64str.c_str(), base64str.length(), (uint8_t*)base64str.c_str(), (size_t)outBufFakeSize, &outBufRealSize);
        if (ret != B64_OUTPUT_BUFFER_TOO_SMALL) {
            Log::error(LOG_TAG, "{}: Geting output buffer size failed: {}", __FUNCTION__, ret);
            return nullptr;
        }

        Log::debug(LOG_TAG, "Decode buffer size: {}", outBufRealSize);

        // Decode buffer
        std::shared_ptr<uint8_t> outBuf(new uint8_t[outBufRealSize]);
        ret = b64Decode(base64str.c_str(), base64str.length(), outBuf.get(), (size_t)outBufRealSize, &outBufRealSize);
        if (ret != B64_OK) {
            Log::error(LOG_TAG, "Decode64 failed {}", ret);
            return nullptr;
        }

        outSize = outBufRealSize;

        return outBuf;
    }

    virtual ~VideoCameraModel() {
        if (mCurFramePtr)
            free(mCurFramePtr);
        mWebCamCap.release();
    }

protected:
    static const constexpr char* LOG_TAG = "VideoCameraModel";

    cv::VideoCapture mWebCamCap;
    cv::Mat mCurFrame;

    size_t mFrameWidth;
    size_t mFrameHeight;
    size_t mBytesPerPixel;
    size_t mFps;
    int mFormat;

    uint8_t* mCurFramePtr;
    size_t mMaxFrameSize;
    std::string mCurFrameBase64;

    std::string mScreenshotsPath;

};

#endif // VIDEOCAMERARESOURCE_H
