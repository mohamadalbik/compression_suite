#include "ImageCodec.h"

Mat compressImg(const Mat img, Mat Q_Y, Mat Q_C) {

	Mat Y(img.rows, img.cols, CV_8UC1);
	Mat Cr(img.rows, img.cols, CV_8UC1);
	Mat Cb(img.rows, img.cols, CV_8UC1);

	BGRtoYCbCr(img, Y, Cr, Cb);

	Mat cb_ds, cr_ds;

	DownSampling420(Cb, Cr, cb_ds, cr_ds);

	PackedBitStream packed_y = encodeChannel(Y, Q_Y);
	PackedBitStream packed_Cr = encodeChannel(cr_ds, Q_C);
	PackedBitStream packed_Cb = encodeChannel(cb_ds, Q_C);

	Mat Y_decoded = decodeChannel(packed_y, Q_Y);
	imshow("y_decoded", Y_decoded);
	Mat Cb_decoded = decodeChannel(packed_Cb, Q_C);
	imshow("Cb_decoded", Cb_decoded);
	Mat Cr_decoded = decodeChannel(packed_Cr, Q_C);
	imshow("Cr_decoded", Cr_decoded);

	resize(Cb_decoded, Cb, Size(Y.cols, Y.rows), 0, 0, INTER_LINEAR);
	resize(Cr_decoded, Cr, Size(Y.cols, Y.rows), 0, 0, INTER_LINEAR);

	std::vector<Mat> channels;

	Mat re;
	channels = { Y_decoded, Cr, Cb };
	merge(channels, re);

	cvtColor(re, re, COLOR_YCrCb2BGR);

	return re;


}