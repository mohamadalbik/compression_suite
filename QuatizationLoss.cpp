#include "JPEGModules.h"

void QuantizationLoss(cv::Mat& residual, const cv::Mat& Q) {
    CV_Assert(residual.rows % 8 == 0 && residual.cols % 8 == 0);
    CV_Assert(residual.type() == CV_32F);


    for (int i = 0; i < residual.rows; i += 8) {
        for (int j = 0; j < residual.cols; j += 8) {
            cv::Mat block = residual(cv::Rect(j, i, 8, 8));

            dct(block, block);
            quantize(block, Q);
            dequantization(block, Q);
            idct(block, block);
        }
    }
}