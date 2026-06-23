#ifndef CONTAINER_FORMAT_H
#define CONTAINER_FORMAT_H

#include <vector>
#include <string>
#include <cstdint>
#include <map>
#include <fstream>
#include "ImageCodec.h"
#include "VideoCodec.h"

#pragma pack(push, 1)

struct SimpleFileHeader {
    char magic[4];          // 'M', 'V', 'I', 'D'
    uint32_t version;       // 1
    uint32_t fps;           // Frames per second
    uint32_t width;
    uint32_t height;
    uint32_t totalFrames;
    uint32_t iFrameInterval;
};

struct SimpleFrameHeader {
    uint32_t frameType;     // 0 = I-frame, 1 = P-frame
    uint32_t ySize;         // Size of Y channel data in bytes
    uint32_t cbSize;        // Size of Cb channel data in bytes
    uint32_t crSize;        // Size of Cr channel data in bytes
    uint32_t mvSize;        // Size of motion vector data (0 for I-frame)
    uint32_t yFreqSize;     // Size of Y frequency table
    uint32_t cbFreqSize;    // Size of Cb frequency table
    uint32_t crFreqSize;    // Size of Cr frequency table
};

#pragma pack(pop)

class VideoContainer {
public:
    static bool writeToFile(const std::string& filename,
        const std::vector<frameData>& frames,
        int fps, int width, int height, int gop);

    static bool readFromFile(const std::string& filename,
        std::vector<frameData>& frames,
        int& fps, int& width, int& height);

private:
    static std::string serializeFrequencyTable(const std::map<int, int>& table);
    static std::map<int, int> deserializeFrequencyTable(const std::string& data);
    static std::string serializeMotionVectors(const MotionField& mv);
    static MotionField deserializeMotionVectors(const std::string& data, int width, int height);
};

#endif