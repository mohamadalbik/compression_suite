#include "JPEGModules.h"

void BGRtoYCbCr(const Mat& img, Mat& Y, Mat& Cr, Mat& Cb) {
    
    Y = Mat(img.rows, img.cols, CV_8UC1);
    Cb = Mat(img.rows, img.cols, CV_8UC1);
    Cr = Mat(img.rows, img.cols, CV_8UC1);

    Vec3b pixel;
    double B, G, R;
    float cr, y, cb;
	for (int i = 0; i < img.rows; i++)
	{
		for(int j = 0; j < img.cols ; j++)
		{
            pixel = img.at<Vec3b>(i, j);

            B = pixel[0];
            G = pixel[1];
            R = pixel[2];

            y = 0.299 * R + 0.587 * G + 0.114 * B ;
            cb = -0.1687 * R - 0.3313 * G + 0.5 * B + 128;
            cr = 0.5 * R - 0.4187 * G - 0.0813 * B + 128;

            Y.at<uchar>(i, j) = saturate_cast<uchar>(y);
            Cr.at<uchar>(i, j) = saturate_cast<uchar>(cr);
            Cb.at<uchar>(i, j) = saturate_cast<uchar>(cb);

		}
	}

}
