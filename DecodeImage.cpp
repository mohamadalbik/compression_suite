#include "ImageCodec.h"

Mat decodeImage(const EncodedImg& encodedImg, cv::Mat Q_Y, cv::Mat Q_C) {
	

	Mat Y_decoded = decodeChannel(encodedImg.packed_y, Q_Y);
	Mat Cr_decoded = decodeChannel(encodedImg.packed_cr, Q_C);
	Mat Cb_decoded = decodeChannel(encodedImg.packed_cb, Q_C);

	resize(Cb_decoded, Cb_decoded, Size(encodedImg.packed_y.width, encodedImg.packed_y.height), 0, 0, INTER_LINEAR);
    resize(Cr_decoded, Cr_decoded, Size(encodedImg.packed_y.width, encodedImg.packed_y.height), 0, 0, INTER_LINEAR);

	std::vector<Mat> channels;

	Mat YCbCr;
	channels = { Y_decoded, Cr_decoded, Cb_decoded };

	merge(channels, YCbCr);

	Mat BGR;
	cvtColor(YCbCr,BGR, COLOR_YCrCb2BGR);

	return BGR;
}