#include "ImageCodec.h"

EncodedImg encodeImage(const Mat& img, cv::Mat Q_Y,  cv::Mat Q_C) {


    EncodedImg result;

	Mat Y(img.rows, img.cols, CV_8UC1);
	Mat Cr(img.rows, img.cols, CV_8UC1);
	Mat Cb(img.rows, img.cols, CV_8UC1);

	BGRtoYCbCr(img, Y, Cr, Cb);
			
	Mat Cr_ds, Cb_ds;
	DownSampling420(Cb, Cr, Cb_ds, Cr_ds);
	result.packed_y = encodeChannel(Y, Q_Y);
	result.packed_cb = encodeChannel(Cb_ds, Q_C);
	result.packed_cr = encodeChannel(Cr_ds, Q_C);
        
    return result;

}