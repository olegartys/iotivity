#include <vector>
#include <string>
#include <atomic>
#include <functional>

#include <OCPlatform.h>
#include <OCApi.h>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include "base64.h"

#include <shouse/Log.h>

#include <dynamic_resource/resource_property.h>

#include <HAL/shouse_res_hal.h>
#include <HAL/shouse_res_hal_dll.h>

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
        return "PkYKOcn3rsipxpWWhz80ald3Pry08YXc6C4gnGwdzV4+ONUtj5jTLMpPCcDArxfQPHxtbZLC/v0XICAsMBz/Q1a1LxLceYrWl+yBOSgwQ/41m6lVQ7+plOjSnU909y0vx5p1+AtyfKYHnNbNvqtpMoeOZTk4HOc18/WvxEgmtRFdxsGBGCg5Nb3hz4lRi4EElycem01VHEVIq8tTlqYXnvynswn2sHJyCeoo+0iNtrksCeD6VwVp8R7CKbyotXUgAF1fjH51oN48FxzbSxNjsOpruhiaa1tY4506tN8sjopXWPXYQWPzr146+lZniWJRqMm44BPAqlYeKrfU9VgEjBGD8L9PetLxSxNyJ124ZBjBrsnKnWw3MtDib5MRc5rUWZVIwcD71cdr5SNw7EMhPRq7HVZDLF5YyCw6gVx3iPZEOF3EHBNeRioOdHSVz0aFVRqXsc5ezW0rmEMQWPQVSu0m2+XCSVX+7Vy5t41la4fCjtzVO4uWkylrKBnqMV5lKPtJe/r+Z3urNSVlp6FKVQiluc9eT0rK1KWOYeU7YOc1qzQpKzIbnOOc4rLvIES4Y7csq8nFawrKLs9uxhXtKygzPv4hGqbV4boQOlZF9M+85HzAdq1LydGZdoJzwB6e9Zt3sWYqGye3oa7Kc6cKl+nY5pRlGkpP+vkYWqJJc20juhzg4ArhtXja/064gdlwFJ";

        mWebCamCap >> mCurFrame;

        Log::debug(LOG_TAG, "Format of read frame: {}", mCurFrame.type());

        if (!mCurFrame.empty()) {
            Log::debug(LOG_TAG, "Read frame, w={} h={} total={} elemsize={}",
                       mCurFrame.cols, mCurFrame.rows,
                       mCurFrame.total(),
                       mCurFrame.elemSize());
            mCurFramePtr = mCurFrame.ptr();

            // cv::imshow("srv", mCurFrame);
            // cv::waitKey(20);

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

    inline size_t width() const { return mFrameWidth; }
    inline size_t height() const { return mFrameHeight; }
    inline size_t bpp() const { return mBytesPerPixel; }
    inline size_t fps() const { return mFps; }

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

extern "C" {
	static constexpr const char* LOG_TAG = "VideoCameraHALDll";
    static VideoCameraModel gCamera;

	int HAL_open() {
        if (!gCamera.init()) {
            return -1;
        }

		return 0;
	}

    ShouseHALResult HAL_close(int id) {
        return ShouseHALResult::OK;
    }
    
    ShouseHALResult HAL_get(int id, const std::string& propName, std::string& resultValue, const OC::QueryParamsMap& params) {
        ShouseHALResult ret = ShouseHALResult::OK;

        Log::info(LOG_TAG, "Get something: {}", propName); 

        if (propName == "width") {
            resultValue = std::to_string(gCamera.width());
        } else if (propName == "height") {
            resultValue = std::to_string(gCamera.height());
        } else if (propName == "bpp") {
            resultValue = std::to_string(gCamera.bpp());
        } else if (propName == "frame") {
            resultValue = gCamera.nextFrameBase64();
        } else {
            ret = ShouseHALResult::ERR;
        }

        return ret;
    }

    ShouseHALResult HAL_put(int id, const std::string& propName, const std::string& newValue, const OC::QueryParamsMap& params) {
        ShouseHALResult ret = ShouseHALResult::OK;

        Log::info(LOG_TAG, "Put something: {}, {}", propName, newValue);

        return ret;        
    }

    void HAL_observe(int id, const OC::QueryParamsMap& params,
        std::atomic_bool& isThreadRunning,
        std::function<void(int)> notifyChanges) {
        int ret = 0;

        while (isThreadRunning) {
            // Log::debug(LOG_TAG, "OBSERVING");
            notifyChanges(ret);
            usleep(gCamera.fps() * 1000);
        }
    }

    std::vector<ResourceProperty> HAL_properties() {
        ResourceProperty prop;
        ResourceProperty prop1;
        ResourceProperty prop2;
        ResourceProperty prop3;

        prop.mName = "width";
        prop.mType = ResourceProperty::Type::T_INT;
        prop.mValue = gCamera.width();

        prop1.mName = "height";
        prop1.mType = ResourceProperty::Type::T_INT;
        prop1.mValue = gCamera.height();

        prop2.mName = "frame";
        prop2.mType = ResourceProperty::Type::T_STRING;
        prop2.mValue = ""; //gCamera.nextFrameBase64();

        prop3.mName = "bpp";
        prop3.mType = ResourceProperty::Type::T_INT;
        prop3.mValue = gCamera.bpp();

        std::vector<ResourceProperty> vec{prop, prop1, prop2, prop3};
        return vec;
    }	
}
