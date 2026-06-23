#include "JPEGModules.h"

#include<opencv2/opencv.hpp>

using namespace cv;

static const int ZigZagOrder[64][2] = {
	{0,0},{0,1},{1,0},{2,0},{1,1},{0,2},{0,3},{1,2},
	{2,1},{3,0},{4,0},{3,1},{2,2},{1,3},{0,4},{0,5},
	{1,4},{2,3},{3,2},{4,1},{5,0},{6,0},{5,1},{4,2},
	{3,3},{2,4},{1,5},{0,6},{0,7},{1,6},{2,5},{3,4},
	{4,3},{5,2},{6,1},{7,0},{7,1},{6,2},{5,3},{4,4},
	{3,5},{2,6},{1,7},{2,7},{3,6},{4,5},{5,4},{6,3},
	{7,2},{7,3},{6,4},{5,5},{4,6},{3,7},{4,7},{5,6},
	{6,5},{7,4},{7,5},{6,6},{5,7},{6,7},{7,6},{7,7}
};


void zigzag8x8(Mat& block, std::vector<int>& zigedBlock) {

	CV_Assert(block.rows == 8 && block.cols == 8);
	CV_Assert(block.type() == CV_32F);


	zigedBlock.clear();
	zigedBlock.reserve(64);
	

	for (int k = 0; k < 64; k++)
	{
		int i = ZigZagOrder[k][0];
		int j = ZigZagOrder[k][1];

		zigedBlock.push_back(static_cast<int>(block.at<float>(i, j)));

	}


}


Mat InverseZigzag8x8(const std::vector<RLES>& rleBlock) {

	Mat block = Mat::zeros(8, 8, CV_32F);
	std::vector<int> coeffs;
	coeffs.reserve(64);

	for (auto& p : rleBlock)
	{

		for (int i = 0; i < p.count; i++)
		{
			coeffs.push_back(p.value);
		}

	}
	CV_Assert(coeffs.size() == 64);
	
	for (int k = 0; k < 64; k++)
	{

		int i = ZigZagOrder[k][0];
		int j = ZigZagOrder[k][1];

		block.at<float>(i, j) = static_cast<float>(coeffs[k]);

	}
	return block;

}