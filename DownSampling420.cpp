#include "JPEGModules.h"

#include<opencv2/opencv.hpp>

using namespace cv;

	void DownSampling420(const Mat& Cb, const Mat& Cr, Mat& Cb_ds, Mat& Cr_ds) {

		int rows = Cb.rows;
		int cols = Cb.cols;

		int rows_ds = rows / 2;
		int cols_ds = cols / 2;

		Cb_ds = Mat(rows_ds, cols_ds, CV_8UC1);
		Cr_ds = Mat(rows_ds, cols_ds, CV_8UC1);

		for (int i = 0; i < rows_ds; i++)
		{
			for (int j = 0; j < cols_ds; j++)
			{

				int r = 2 * i;
				int c = 2 * j;

				int cb_sum = Cb.at<uchar>(r, c) +
					Cb.at<uchar>(r + 1, c) +
					Cb.at<uchar>(r, c + 1) +
					Cb.at<uchar>(r + 1, c + 1);

				int cr_sum = Cr.at<uchar>(r, c) +
					Cr.at<uchar>(r + 1, c) +
					Cr.at<uchar>(r, c + 1) +
					Cr.at<uchar>(r + 1, c + 1);

				Cb_ds.at<uchar>(i, j) = static_cast<uchar>(cb_sum / 4);
				Cr_ds.at<uchar>(i, j) = static_cast<uchar>(cr_sum / 4);

			}
		}


}

