#ifndef JPEG_Modules_H
#define JPEG_Modules_H

#include<opencv2/opencv.hpp>
#include"RLE.h"

using namespace cv;

/* ===============================
   Color Space Conversion
   =============================== */

void BGRtoYCbCr(const Mat& img, Mat& Y, Mat& Cr, Mat& Cb);

/* ===============================
   Chroma Downsampling
   =============================== */

void DownSampling420(const Mat& Cb, const Mat& Cr, Mat& Cb_ds, Mat& Cr_ds);

/* ===============================
   8x8Block Splitting
   =============================== */

void _8x8Block(const Mat& channel, std::vector<Mat>& blocks);

/* ===============================
   DCT2 8x8Blocks
   =============================== */

void DCT2(Mat& block);

/* ===============================
   IDCT1 8x8Blocks
   =============================== */

void IDCT1D(const float* input, float* output);

/* ===============================
   IDCT2 8x8Blocks
   =============================== */

void IDCT2D(Mat& block);


/* ===============================
   Quantization
   =============================== */

void quantize(Mat& block, const Mat Q);

/* ===============================
   ZigZag 8x8
   =============================== */

void zigzag8x8(Mat& block, std::vector<int>& zigedBlock);

/* ===============================
   Inverse ZigZag 8x8
   =============================== */

Mat InverseZigzag8x8(const std::vector<RLES>& rleBlock);

/* ===============================
   Dequantization
   =============================== */

void dequantization(Mat& block, const Mat Q);

/* ===============================
   Reconstruc Image
   =============================== */

void reconstruct(Mat& block, const int rows, const int cols, Mat& reconstructed);

/* ===============================
   PSNR
   =============================== */

double PSNR(const Mat& original, const Mat& reconstructed);

/* ===============================
   Compression Ratio
   =============================== */

double compressionRatio(const Mat& original, const Mat& reconstructed);

/* ===============================
   SSIM
   =============================== */

double ssim(const Mat& original, const Mat& reconstructed);

/* ===============================
   spatial Compression
   =============================== */

void QuantizationLoss(cv::Mat& residual, const cv::Mat& Q);

#endif // !JPEG_Modules_H
