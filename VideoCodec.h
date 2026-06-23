#ifndef VIDEO_CODEC
#define VIDEO_CODEC

#include<opencv2/opencv.hpp>
#include<string>
#include<vector>
#include<iostream>
#include"ImageCodec.h"
#include"MotionEstimation.h"

/* ===============================
   Frame Struct
   =============================== */

struct frameData
{

	EncodedImg encodedFrame;
	bool isIFrame;
	bool refreshChroma;

	MotionField mv;
	bool hasMotionVectors;

};


struct EncodeSettings {
    double targetFps;
    int targetWidth;
    int targetHeight;
    double qualityFactor;
    bool resizeEnabled;
    bool fpsChangeEnabled;
};


/* ===============================
   Frame Extraction
   =============================== */

void extractFrame(const std::string videoPath, std::vector<cv::Mat>& frames, int& fps);

/* ===============================
   Video Encoding 
   =============================== */

void encodeVideo(const std::string videoPath, std::vector<frameData>& encodedFrames, int& fps, double quality);

/* ===============================
   Video Decoding
   =============================== */

void decodeVideo(const std::string outputFileName ,const std::vector<frameData>& encodedFrames, int& fps, double quality);


/* ===============================
   Compute Residual
   =============================== */

void computeResidual(const cv::Mat& current, const cv::Mat& refrence, cv::Mat& residual);

/* ===============================
   Reconstruct Residual
   =============================== */

void reconstructResidual(const cv::Mat& current, const cv::Mat& refrence, cv::Mat& residual);

void encodeVideoToFile(const std::string inputVideoPath, const std::string outputFilePath, double quality);

void decodeVideoFromFile(const std::string inputFilePath, const std::string outputFileName, double quality);

void encodeVideoWithSettings(const std::string videoPath,
                             std::vector<frameData>& encodedFrames,
                             int& fps,
                             const EncodeSettings& settings);

#endif // !VIDEO_CODEC

