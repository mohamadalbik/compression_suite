#include"VideoCodec.h"


void reconstructResidual(const cv::Mat& residual, const cv::Mat& refrence, cv::Mat& reconstructed) {

	cv::Mat ref32, res32;



	refrence.convertTo(ref32, CV_32FC1);
	residual.convertTo(res32, CV_32FC1);


	cv::Mat temp;

	cv::add(ref32, res32, temp);

	temp.convertTo(reconstructed, CV_8UC1);

}
