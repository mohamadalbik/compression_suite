#include "JPEGModules.h"

#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void _8x8Block(const Mat& channel, vector<Mat>& blocks)
{

	blocks.clear();

	int rows = channel.rows;
	int cols = channel.cols;

	for (int i = 0; i < rows; i+=8)
	{
		for (int j = 0; j < cols; j+=8)
		{
			Mat block = channel(Rect(j, i, 8, 8)).clone();

			blocks.push_back(block);
		}
	}

}