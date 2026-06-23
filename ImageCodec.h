#ifndef IMAGE_CODEC
#define IMAGE_CODEC

#include"JPEGModules.h"
#include"Huffman.h"
#include"RLE.h"
#include"BitPacking.h"

using namespace cv;

/* ===============================
   Encoded Image Struct
   =============================== */

struct EncodedImg
{

	PackedBitStream packed_y;
	PackedBitStream packed_cr;
	PackedBitStream packed_cb;

};

/* ===============================
   Endcode Channel
   =============================== */

PackedBitStream encodeChannel(Mat channel, const Mat Q);

/* ===============================
   Endcode Image
   =============================== */

EncodedImg encodeImage(const Mat& img, cv::Mat Q_Y, cv::Mat Q_C);

/* ===============================
   Decode Channel
   =============================== */

Mat decodeChannel(PackedBitStream encodedChannel, const Mat Q);

/* ===============================
   Decode Image
   =============================== */

Mat decodeImage(const EncodedImg& encodedImg, cv::Mat Q_Y, cv::Mat Q_C);

/* ===============================
   Image compression
   =============================== */

Mat compressImg(const Mat img, Mat Q_Y, Mat Q_C);



#endif // !IMAGE_CODEC

