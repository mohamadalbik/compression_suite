#include "JPEGModules.h"

#include<opencv2/opencv.hpp>
#include<cmath>

constexpr auto PI = 3.14159265358979323846;


using namespace std;
using namespace cv;

void DCT2(Mat& block) {
	Mat temp;

	block.convertTo(temp, CV_32F);

	temp = temp - 128;

	Mat dct2Block = Mat::zeros(8, 8, CV_32F);

	for (int u = 0; u < 8; u++)
	{
		for(int v = 0; v < 8; v++)
		{
			
			float sum = 0.0;

			for (int x = 0; x < 8; x++)
			{
				for(int y = 0; y < 8; y++)
				{
					sum += temp.at<float>(x, y) *
						cos((2 * x + 1) * u * PI / 16) *
						cos((2 * y + 1) * v * PI / 16);
				}
			}

			float Cu = (u == 0) ? (1.0 / sqrt(2.0)) : 1.0;
			float Cv = (u == 0) ? (1.0 / sqrt(2.0)) : 1.0;

			dct2Block.at<float>(u, v) = 0.25 * Cu * Cv * sum;
		}
	}

	dct2Block.copyTo(block);
	

}