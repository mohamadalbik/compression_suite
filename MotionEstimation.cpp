#include "MotionEstimation.h"
#include <algorithm>
#include <cmath>
#include <climits>

static const int DIAMOND_PATTERN_LARGE[9][2] = {
    {0, 0}, {0, -2}, {1, -1}, {2, 0}, {1, 1},
    {0, 2}, {-1, 1}, {-2, 0}, {-1, -1}
};

static const int DIAMOND_PATTERN_SMALL[5][2] = {
    {0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}
};

MotionEstimator::MotionEstimator(int bs, int sr, bool subpixel, int algo)
    : blockSize(bs), searchRange(sr), useSubpixel(subpixel), algorithm(algo) {
    if (blockSize < 4 || blockSize > 64) blockSize = 16;
    if (searchRange < 4 || searchRange > 128) searchRange = 16;
    if (algorithm < 0 || algorithm > 2) algorithm = 1;
}

int MotionEstimator::computeSAD(const cv::Mat& block1, const cv::Mat& block2) {
    cv::Mat diff;
    cv::absdiff(block1, block2, diff);
    cv::Scalar sum = cv::sum(diff);
    return static_cast<int>(sum[0]);
}

MotionVector MotionEstimator::diamondSearch(const cv::Mat& currBlock,
    const cv::Mat& refFrame,
    int startX, int startY) {
    int bestX = 0, bestY = 0;

    int refX = std::max(0, std::min(startX, refFrame.cols - blockSize));
    int refY = std::max(0, std::min(startY, refFrame.rows - blockSize));
    cv::Mat refBlock = refFrame(cv::Rect(refX, refY, blockSize, blockSize));
    int bestSAD = computeSAD(currBlock, refBlock);

    int step = 2;
    bool improved = true;

    while (improved && step >= 1) {
        improved = false;

        if (step == 2) {
            for (int i = 0; i < 9; i++) {
                int dx = DIAMOND_PATTERN_LARGE[i][0];
                int dy = DIAMOND_PATTERN_LARGE[i][1];

                int candidateX = bestX + dx;
                int candidateY = bestY + dy;

                if (std::abs(candidateX) > searchRange ||
                    std::abs(candidateY) > searchRange) {
                    continue;
                }

                int refX = startX + candidateX;
                int refY = startY + candidateY;

                if (refX < 0 || refY < 0 ||
                    refX + blockSize > refFrame.cols ||
                    refY + blockSize > refFrame.rows) {
                    continue;
                }

                cv::Mat candidateBlock = refFrame(cv::Rect(refX, refY, blockSize, blockSize));
                int sad = computeSAD(currBlock, candidateBlock);

                if (sad < bestSAD) {
                    bestSAD = sad;
                    bestX = candidateX;
                    bestY = candidateY;
                    improved = true;
                }
            }
        }
        else {
            for (int i = 0; i < 5; i++) {
                int dx = DIAMOND_PATTERN_SMALL[i][0];
                int dy = DIAMOND_PATTERN_SMALL[i][1];

                int candidateX = bestX + dx;
                int candidateY = bestY + dy;

                if (std::abs(candidateX) > searchRange ||
                    std::abs(candidateY) > searchRange) {
                    continue;
                }

                int refX = startX + candidateX;
                int refY = startY + candidateY;

                if (refX < 0 || refY < 0 ||
                    refX + blockSize > refFrame.cols ||
                    refY + blockSize > refFrame.rows) {
                    continue;
                }

                cv::Mat candidateBlock = refFrame(cv::Rect(refX, refY, blockSize, blockSize));
                int sad = computeSAD(currBlock, candidateBlock);

                if (sad < bestSAD) {
                    bestSAD = sad;
                    bestX = candidateX;
                    bestY = candidateY;
                    improved = true;
                }
            }
        }

        if (!improved) {
            if (step == 2) {
                step = 1;
                improved = true;
            }
            else {
                step = 0;
            }
        }
    }

    return MotionVector(static_cast<short>(bestX), static_cast<short>(bestY),
        static_cast<unsigned short>(bestSAD));
}

MotionVector MotionEstimator::threeStepSearch(const cv::Mat& currBlock,
    const cv::Mat& refFrame,
    int startX, int startY) {
    int bestX = 0, bestY = 0;

    int refX = std::max(0, std::min(startX, refFrame.cols - blockSize));
    int refY = std::max(0, std::min(startY, refFrame.rows - blockSize));
    cv::Mat refBlock = refFrame(cv::Rect(refX, refY, blockSize, blockSize));
    int bestSAD = computeSAD(currBlock, refBlock);

    int step = searchRange / 2;

    while (step >= 1) {
        bool improved = false;

        for (int dy = -step; dy <= step; dy += step) {
            for (int dx = -step; dx <= step; dx += step) {
                int candidateX = bestX + dx;
                int candidateY = bestY + dy;

                if (std::abs(candidateX) > searchRange ||
                    std::abs(candidateY) > searchRange) {
                    continue;
                }

                int refX = startX + candidateX;
                int refY = startY + candidateY;

                if (refX < 0 || refY < 0 ||
                    refX + blockSize > refFrame.cols ||
                    refY + blockSize > refFrame.rows) {
                    continue;
                }

                cv::Mat candidateBlock = refFrame(cv::Rect(refX, refY, blockSize, blockSize));
                int sad = computeSAD(currBlock, candidateBlock);

                if (sad < bestSAD) {
                    bestSAD = sad;
                    bestX = candidateX;
                    bestY = candidateY;
                    improved = true;
                }
            }
        }

        if (!improved) {
            step /= 2;
        }
    }

    return MotionVector(static_cast<short>(bestX), static_cast<short>(bestY),
        static_cast<unsigned short>(bestSAD));
}

MotionVector MotionEstimator::fullSearch(const cv::Mat& currBlock,
    const cv::Mat& refFrame,
    int startX, int startY) {
    int bestX = 0, bestY = 0;
    int bestSAD = INT_MAX;

    for (int dy = -searchRange; dy <= searchRange; dy++) {
        for (int dx = -searchRange; dx <= searchRange; dx++) {
            int refX = startX + dx;
            int refY = startY + dy;

            if (refX < 0 || refY < 0 ||
                refX + blockSize > refFrame.cols ||
                refY + blockSize > refFrame.rows) {
                continue;
            }

            cv::Mat candidateBlock = refFrame(cv::Rect(refX, refY, blockSize, blockSize));
            int sad = computeSAD(currBlock, candidateBlock);

            if (sad < bestSAD) {
                bestSAD = sad;
                bestX = dx;
                bestY = dy;
            }
        }
    }

    return MotionVector(static_cast<short>(bestX), static_cast<short>(bestY),
        static_cast<unsigned short>(bestSAD));
}

cv::Mat MotionEstimator::interpolateHalfPixel(const cv::Mat& frame, float x, float y, int size) {
    cv::Mat block(size, size, CV_8UC1);

    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    float fx = x - ix;
    float fy = y - iy;

    for (int by = 0; by < size; by++) {
        for (int bx = 0; bx < size; bx++) {
            int x0 = ix + bx;
            int y0 = iy + by;
            int x1 = x0 + 1;
            int y1 = y0 + 1;

            if (x1 >= frame.cols) x1 = x0;
            if (y1 >= frame.rows) y1 = y0;

            float p00 = frame.at<uchar>(y0, x0);
            float p10 = frame.at<uchar>(y0, x1);
            float p01 = frame.at<uchar>(y1, x0);
            float p11 = frame.at<uchar>(y1, x1);

            float value = (1.0f - fx) * (1.0f - fy) * p00 +
                fx * (1.0f - fy) * p10 +
                (1.0f - fx) * fy * p01 +
                fx * fy * p11;

            block.at<uchar>(by, bx) = static_cast<uchar>(std::round(value));
        }
    }

    return block;
}

void MotionEstimator::estimate(const cv::Mat& current, const cv::Mat& reference,
    MotionField& mv, cv::Mat& residual) {
    CV_Assert(current.size() == reference.size());
    CV_Assert(current.type() == CV_8UC1);
    CV_Assert(reference.type() == CV_8UC1);

    mv.blockSize = blockSize;
    mv.width = current.cols;
    mv.height = current.rows;
    mv.cols = (current.cols + blockSize - 1) / blockSize;
    mv.rows = (current.rows + blockSize - 1) / blockSize;
    mv.vectors.resize(mv.cols * mv.rows);

    residual = cv::Mat::zeros(current.size(), CV_32F);

    cv::Mat currFloat;
    current.convertTo(currFloat, CV_32F);

    for (int by = 0; by < mv.rows; by++) {
        int y = by * blockSize;
        int blockH = std::min(blockSize, current.rows - y);

        for (int bx = 0; bx < mv.cols; bx++) {
            int x = bx * blockSize;
            int blockW = std::min(blockSize, current.cols - x);

            cv::Mat currBlock = current(cv::Rect(x, y, blockW, blockH));

            MotionVector vec;
            switch (algorithm) {
            case 0:
                vec = fullSearch(currBlock, reference, x, y);
                break;
            case 1:
                vec = diamondSearch(currBlock, reference, x, y);
                break;
            case 2:
                vec = threeStepSearch(currBlock, reference, x, y);
                break;
            default:
                vec = diamondSearch(currBlock, reference, x, y);
            }

            mv.vectors[by * mv.cols + bx] = vec;

            int refX = x + vec.x;
            int refY = y + vec.y;

            refX = std::max(0, std::min(refX, reference.cols - blockW));
            refY = std::max(0, std::min(refY, reference.rows - blockH));

            cv::Mat refBlock = reference(cv::Rect(refX, refY, blockW, blockH));
            cv::Mat refBlockFloat;
            refBlock.convertTo(refBlockFloat, CV_32F);

            cv::Mat currBlockFloat = currFloat(cv::Rect(x, y, blockW, blockH));

            cv::Mat blockResidual;
            cv::subtract(currBlockFloat, refBlockFloat, blockResidual);
            blockResidual.copyTo(residual(cv::Rect(x, y, blockW, blockH)));
        }
    }

    
}

void MotionEstimator::compensate(const cv::Mat& reference, const MotionField& mv,
    cv::Mat& compensated) {
    compensated = cv::Mat::zeros(reference.size(), CV_8UC1);

    for (int by = 0; by < mv.rows; by++) {
        int y = by * mv.blockSize;
        int blockH = std::min(mv.blockSize, reference.rows - y);

        for (int bx = 0; bx < mv.cols; bx++) {
            int x = bx * mv.blockSize;
            int blockW = std::min(mv.blockSize, reference.cols - x);

            const MotionVector& vec = mv.getVector(bx, by);

            int refX = x + vec.x;
            int refY = y + vec.y;

            refX = std::max(0, std::min(refX, reference.cols - blockW));
            refY = std::max(0, std::min(refY, reference.rows - blockH));

            cv::Mat refBlock = reference(cv::Rect(refX, refY, blockW, blockH));
            refBlock.copyTo(compensated(cv::Rect(x, y, blockW, blockH)));
        }
    }
}

static std::string encodeExpGolomb(short value) {
    unsigned int u;
    if (value > 0) {
        u = 2 * value - 1;
    }
    else {
        u = -2 * value;
    }

    unsigned int temp = u + 1;
    int bitLen = 0;
    while (temp > 0) {
        temp >>= 1;
        bitLen++;
    }

    std::string code;
    for (int i = 0; i < bitLen - 1; i++) {
        code += '0';
    }

    temp = u + 1;
    for (int i = bitLen - 1; i >= 0; i--) {
        code += ((temp >> i) & 1) ? '1' : '0';
    }

    return code;
}

static short decodeExpGolomb(const std::string& bits, size_t& pos) {
    int zeros = 0;
    while (pos < bits.size() && bits[pos] == '0') {
        zeros++;
        pos++;
    }

    if (pos >= bits.size()) return 0;

    pos++;

    unsigned int value = 1;
    for (int i = 0; i < zeros; i++) {
        value <<= 1;
        if (pos < bits.size() && bits[pos] == '1') {
            value |= 1;
        }
        pos++;
    }

    unsigned int u = value - 1;

    if (u % 2 == 0) {
        return -static_cast<short>(u / 2);
    }
    else {
        return static_cast<short>((u + 1) / 2);
    }
}

static short median(short a, short b, short c) {
    if (a > b) std::swap(a, b);
    if (b > c) std::swap(b, c);
    if (a > b) std::swap(a, b);
    return b;
}

std::string encodeMotionVectors(const MotionField& mv) {
    std::string bitstream;

    bitstream += static_cast<char>(mv.blockSize);

    bitstream += static_cast<char>((mv.cols >> 8) & 0xFF);
    bitstream += static_cast<char>(mv.cols & 0xFF);
    bitstream += static_cast<char>((mv.rows >> 8) & 0xFF);
    bitstream += static_cast<char>(mv.rows & 0xFF);

    bitstream += static_cast<char>((mv.width >> 8) & 0xFF);
    bitstream += static_cast<char>(mv.width & 0xFF);
    bitstream += static_cast<char>((mv.height >> 8) & 0xFF);
    bitstream += static_cast<char>(mv.height & 0xFF);

    for (int by = 0; by < mv.rows; by++) {
        for (int bx = 0; bx < mv.cols; bx++) {
            const MotionVector& curr = mv.getVector(bx, by);

            MotionVector pred(0, 0);

            bool hasLeft = (bx > 0);
            bool hasAbove = (by > 0);
            bool hasAboveRight = (by > 0 && bx < mv.cols - 1);

            if (hasLeft && hasAbove && hasAboveRight) {
                const MotionVector& left = mv.getVector(bx - 1, by);
                const MotionVector& above = mv.getVector(bx, by - 1);
                const MotionVector& aboveRight = mv.getVector(bx + 1, by - 1);

                short predX = median(left.x, above.x, aboveRight.x);
                short predY = median(left.y, above.y, aboveRight.y);
                pred = MotionVector(predX, predY);
            }
            else if (hasLeft && hasAbove) {
                const MotionVector& left = mv.getVector(bx - 1, by);
                const MotionVector& above = mv.getVector(bx, by - 1);
                pred = MotionVector((left.x + above.x) / 2, (left.y + above.y) / 2);
            }
            else if (hasLeft) {
                pred = mv.getVector(bx - 1, by);
            }
            else if (hasAbove) {
                pred = mv.getVector(bx, by - 1);
            }

            short mvdX = curr.x - pred.x;
            short mvdY = curr.y - pred.y;

            bitstream += encodeExpGolomb(mvdX);
            bitstream += encodeExpGolomb(mvdY);
        }
    }

    return bitstream;
}

MotionField decodeMotionVectors(const std::string& bitstream, int cols, int rows,
    int width, int height, int blockSize) {
    MotionField mv;
    mv.blockSize = blockSize;
    mv.cols = cols;
    mv.rows = rows;
    mv.width = width;
    mv.height = height;
    mv.vectors.resize(cols * rows);

    size_t pos = 0;

    for (int by = 0; by < rows; by++) {
        for (int bx = 0; bx < cols; bx++) {
            MotionVector pred(0, 0);

            bool hasLeft = (bx > 0);
            bool hasAbove = (by > 0);
            bool hasAboveRight = (by > 0 && bx < cols - 1);

            if (hasLeft && hasAbove && hasAboveRight) {
                const MotionVector& left = mv.getVector(bx - 1, by);
                const MotionVector& above = mv.getVector(bx, by - 1);
                const MotionVector& aboveRight = mv.getVector(bx + 1, by - 1);

                short predX = median(left.x, above.x, aboveRight.x);
                short predY = median(left.y, above.y, aboveRight.y);
                pred = MotionVector(predX, predY);
            }
            else if (hasLeft && hasAbove) {
                const MotionVector& left = mv.getVector(bx - 1, by);
                const MotionVector& above = mv.getVector(bx, by - 1);
                pred = MotionVector((left.x + above.x) / 2, (left.y + above.y) / 2);
            }
            else if (hasLeft) {
                pred = mv.getVector(bx - 1, by);
            }
            else if (hasAbove) {
                pred = mv.getVector(bx, by - 1);
            }

            short mvdX = decodeExpGolomb(bitstream, pos);
            short mvdY = decodeExpGolomb(bitstream, pos);

            MotionVector curr;
            curr.x = pred.x + mvdX;
            curr.y = pred.y + mvdY;
            curr.sad = 0;

            mv.vectors[by * cols + bx] = curr;
        }
    }

    return mv;
}