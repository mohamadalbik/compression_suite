#include "JPEGModules.h"

#include<cmath>

constexpr auto PI = 3.14159265358979323846;

void IDCT2D(Mat& block) {
    CV_Assert(block.rows == 8 && block.cols == 8);
    CV_Assert(block.type() == CV_32F);

    cv::Mat temp(8, 8, CV_32F);

    for (int i = 0; i < 8; i++) {
        float row[8], transformed[8];

        for (int j = 0; j < 8; j++) {
            row[j] = block.at<float>(i, j);
        }

        IDCT1D(row, transformed);

        for (int j = 0; j < 8; j++) {
            temp.at<float>(j, i) = transformed[j];
        }
    }

    for (int i = 0; i < 8; i++) {
        float column[8], transformed[8];

        for (int j = 0; j < 8; j++) {
            column[j] = temp.at<float>(i, j);
        }

        IDCT1D(column, transformed);

        for (int j = 0; j < 8; j++) {
            block.at<float>(j, i) = transformed[j];
        }
    }
    block+=128;
}