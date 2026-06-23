#ifndef MOTION_ESTIMATION_H
#define MOTION_ESTIMATION_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <cstdint>

struct MotionVector {
    short x;
    short y;
    unsigned short sad;

    MotionVector(short _x = 0, short _y = 0, unsigned short _sad = 65535)
        : x(_x), y(_y), sad(_sad) {}
};

struct MotionField {
    std::vector<MotionVector> vectors;
    int blockSize;
    int cols;
    int rows;
    int width;
    int height;

    MotionVector& getVector(int blockX, int blockY) {
        return vectors[blockY * cols + blockX];
    }

    const MotionVector& getVector(int blockX, int blockY) const {
        return vectors[blockY * cols + blockX];
    }
};

class MotionEstimator {
public:
    MotionEstimator(int blockSize = 16, int searchRange = 16,
        bool useSubpixel = false, int algorithm = 1);

    void estimate(const cv::Mat& current, const cv::Mat& reference,
        MotionField& mv, cv::Mat& residual);

    void compensate(const cv::Mat& reference, const MotionField& mv,
        cv::Mat& compensated);

    int getBlockSize() const { return blockSize; }
    int getSearchRange() const { return searchRange; }

private:
    int blockSize;
    int searchRange;
    bool useSubpixel;
    int algorithm;

    int computeSAD(const cv::Mat& block1, const cv::Mat& block2);
    MotionVector diamondSearch(const cv::Mat& currBlock, const cv::Mat& refFrame,
        int startX, int startY);
    MotionVector threeStepSearch(const cv::Mat& currBlock, const cv::Mat& refFrame,
        int startX, int startY);
    MotionVector fullSearch(const cv::Mat& currBlock, const cv::Mat& refFrame,
        int startX, int startY);
    cv::Mat interpolateHalfPixel(const cv::Mat& frame, float x, float y, int size);
};

std::string encodeMotionVectors(const MotionField& mv);
MotionField decodeMotionVectors(const std::string& bitstream, int cols, int rows,
    int width, int height, int blockSize);

#endif