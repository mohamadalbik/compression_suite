#include "JPEGModules.h"
#include<opencv2/opencv.hpp>

using namespace cv;

void reconstruct(Mat& block,const int row,const int col, Mat& reconstructed) {

	block.copyTo(reconstructed(Rect(col, row, 8, 8)));

}