#include "VideoCodec.h"

void computeResidual(const cv::Mat& current, const cv::Mat& reference, cv::Mat& residual) {

	cv::Mat curr32, ref32;

	current.convertTo(curr32, CV_32FC1);
	reference.convertTo(ref32, CV_32FC1);

	cv::subtract(curr32, ref32, residual);

}