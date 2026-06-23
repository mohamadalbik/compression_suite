#include "ContainerFormat.h"
#include <iostream>
#include <cstring>

std::string VideoContainer::serializeFrequencyTable(const std::map<int, int>& table) {
    std::string data;

    uint32_t size = static_cast<uint32_t>(table.size());
    data.append(reinterpret_cast<const char*>(&size), sizeof(uint32_t));

    for (const auto& entry : table) {
        int symbol = entry.first;
        int freq = entry.second;
        data.append(reinterpret_cast<const char*>(&symbol), sizeof(int));
        data.append(reinterpret_cast<const char*>(&freq), sizeof(int));
    }

    return data;
}

std::map<int, int> VideoContainer::deserializeFrequencyTable(const std::string& data) {
    std::map<int, int> table;

    if (data.size() < sizeof(uint32_t)) return table;

    uint32_t size;
    memcpy(&size, data.data(), sizeof(uint32_t));

    size_t pos = sizeof(uint32_t);
    for (uint32_t i = 0; i < size && pos + 2 * sizeof(int) <= data.size(); i++) {
        int symbol, freq;
        memcpy(&symbol, data.data() + pos, sizeof(int));
        pos += sizeof(int);
        memcpy(&freq, data.data() + pos, sizeof(int));
        pos += sizeof(int);
        table[symbol] = freq;
    }

    return table;
}

std::string VideoContainer::serializeMotionVectors(const MotionField& mv) {
    std::string data;

    uint32_t numVectors = static_cast<uint32_t>(mv.vectors.size());
    data.append(reinterpret_cast<const char*>(&numVectors), sizeof(uint32_t));

    for (const auto& vec : mv.vectors) {
        data.append(reinterpret_cast<const char*>(&vec.x), sizeof(short));
        data.append(reinterpret_cast<const char*>(&vec.y), sizeof(short));
    }

    return data;
}

MotionField VideoContainer::deserializeMotionVectors(const std::string& data, int width, int height) {
    MotionField mv;

    if (data.size() < sizeof(uint32_t)) return mv;

    uint32_t numVectors;
    memcpy(&numVectors, data.data(), sizeof(uint32_t));

    mv.vectors.reserve(numVectors);
    mv.blockSize = 16;
    mv.cols = (width + 15) / 16;
    mv.rows = (height + 15) / 16;
    mv.width = width;
    mv.height = height;

    size_t pos = sizeof(uint32_t);
    for (uint32_t i = 0; i < numVectors && pos + 2 * sizeof(short) <= data.size(); i++) {
        MotionVector vec;
        memcpy(&vec.x, data.data() + pos, sizeof(short));
        pos += sizeof(short);
        memcpy(&vec.y, data.data() + pos, sizeof(short));
        pos += sizeof(short);
        vec.sad = 0;
        mv.vectors.push_back(vec);
    }

    return mv;
}

// Helper functions to serialize/deserialize PackedBitStream
static std::string serializePackedStream(const PackedBitStream& stream) {
    std::string data;

    // Store dimensions
    data.append(reinterpret_cast<const char*>(&stream.width), sizeof(int));
    data.append(reinterpret_cast<const char*>(&stream.height), sizeof(int));

    // Store validBits
    data.append(reinterpret_cast<const char*>(&stream.validBits), sizeof(int));

    // Store frequency table
    uint32_t freqSize = static_cast<uint32_t>(stream.frequencyTable.size());
    data.append(reinterpret_cast<const char*>(&freqSize), sizeof(uint32_t));

    for (const auto& entry : stream.frequencyTable) {
        data.append(reinterpret_cast<const char*>(&entry.first), sizeof(int));
        data.append(reinterpret_cast<const char*>(&entry.second), sizeof(int));
    }

    // Store compressed data
    uint32_t dataSize = static_cast<uint32_t>(stream.data.size());
    data.append(reinterpret_cast<const char*>(&dataSize), sizeof(uint32_t));
    data.append(reinterpret_cast<const char*>(stream.data.data()), dataSize);

    return data;
}

static PackedBitStream deserializePackedStream(const std::string& data) {
    PackedBitStream stream;
    size_t pos = 0;

    if (data.size() < sizeof(int) * 2 + sizeof(int) + sizeof(uint32_t)) {
        return stream;
    }

    memcpy(&stream.width, data.data() + pos, sizeof(int));
    pos += sizeof(int);
    memcpy(&stream.height, data.data() + pos, sizeof(int));
    pos += sizeof(int);
    memcpy(&stream.validBits, data.data() + pos, sizeof(int));
    pos += sizeof(int);

    uint32_t freqSize;
    memcpy(&freqSize, data.data() + pos, sizeof(uint32_t));
    pos += sizeof(uint32_t);

    for (uint32_t i = 0; i < freqSize && pos + 2 * sizeof(int) <= data.size(); i++) {
        int symbol, freq;
        memcpy(&symbol, data.data() + pos, sizeof(int));
        pos += sizeof(int);
        memcpy(&freq, data.data() + pos, sizeof(int));
        pos += sizeof(int);
        stream.frequencyTable[symbol] = freq;
    }

    uint32_t dataSize;
    memcpy(&dataSize, data.data() + pos, sizeof(uint32_t));
    pos += sizeof(uint32_t);

    if (pos + dataSize <= data.size()) {
        stream.data.resize(dataSize);
        memcpy(stream.data.data(), data.data() + pos, dataSize);
    }

    return stream;
}

bool VideoContainer::writeToFile(const std::string& filename,
    const std::vector<frameData>& frames,
    int fps, int width, int height, int gop) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot create file: " << filename << std::endl;
        return false;
    }

    // Write header
    SimpleFileHeader header;
    header.magic[0] = 'M'; header.magic[1] = 'V';
    header.magic[2] = 'I'; header.magic[3] = 'D';
    header.version = 1;
    header.fps = fps;
    header.width = width;
    header.height = height;
    header.totalFrames = static_cast<uint32_t>(frames.size());
    header.iFrameInterval = gop;

    file.write(reinterpret_cast<const char*>(&header), sizeof(SimpleFileHeader));

    // Write each frame
    for (const auto& frame : frames) {
        SimpleFrameHeader frameHeader;
        frameHeader.frameType = frame.isIFrame ? 0 : 1;

        // Get serialized data
        std::string yData = serializePackedStream(frame.encodedFrame.packed_y);
        std::string cbData = serializePackedStream(frame.encodedFrame.packed_cb);
        std::string crData = serializePackedStream(frame.encodedFrame.packed_cr);

        frameHeader.ySize = static_cast<uint32_t>(yData.size());
        frameHeader.cbSize = static_cast<uint32_t>(cbData.size());
        frameHeader.crSize = static_cast<uint32_t>(crData.size());

        std::string mvData;
        if (!frame.isIFrame) {
            mvData = serializeMotionVectors(frame.mv);
        }
        frameHeader.mvSize = static_cast<uint32_t>(mvData.size());

        // Extract frequency table sizes from the packed streams
        // The frequency tables are included in the serialized data

        file.write(reinterpret_cast<const char*>(&frameHeader), sizeof(SimpleFrameHeader));
        file.write(yData.data(), yData.size());
        file.write(cbData.data(), cbData.size());
        file.write(crData.data(), crData.size());
        if (!mvData.empty()) {
            file.write(mvData.data(), mvData.size());
        }
    }

    file.close();
    std::cout << "Written " << frames.size() << " frames to " << filename << std::endl;
    return true;
}

bool VideoContainer::readFromFile(const std::string& filename,
    std::vector<frameData>& frames,
    int& fps, int& width, int& height) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }

    // Read header
    SimpleFileHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(SimpleFileHeader));

    if (header.magic[0] != 'M' || header.magic[1] != 'V' ||
        header.magic[2] != 'I' || header.magic[3] != 'D') {
        std::cerr << "Invalid file format" << std::endl;
        return false;
    }

    fps = header.fps;
    width = header.width;
    height = header.height;

    frames.clear();
    frames.reserve(header.totalFrames);

    for (uint32_t i = 0; i < header.totalFrames; i++) {
        SimpleFrameHeader frameHeader;
        file.read(reinterpret_cast<char*>(&frameHeader), sizeof(SimpleFrameHeader));

        frameData f;
        f.isIFrame = (frameHeader.frameType == 0);

        // Read Y data
        std::string yData(frameHeader.ySize, '\0');
        file.read(&yData[0], frameHeader.ySize);
        f.encodedFrame.packed_y = deserializePackedStream(yData);

        // Read Cb data
        std::string cbData(frameHeader.cbSize, '\0');
        file.read(&cbData[0], frameHeader.cbSize);
        f.encodedFrame.packed_cb = deserializePackedStream(cbData);

        // Read Cr data
        std::string crData(frameHeader.crSize, '\0');
        file.read(&crData[0], frameHeader.crSize);
        f.encodedFrame.packed_cr = deserializePackedStream(crData);

        // Read motion vectors for P-frames
        if (!f.isIFrame && frameHeader.mvSize > 0) {
            std::string mvData(frameHeader.mvSize, '\0');
            file.read(&mvData[0], frameHeader.mvSize);
            f.mv = deserializeMotionVectors(mvData, width, height);
            f.hasMotionVectors = true;
        }
        else {
            f.hasMotionVectors = false;
        }

        frames.push_back(f);

        // Progress update
        if ((i + 1) % 50 == 0) {
            std::cout << "Loaded " << (i + 1) << "/" << header.totalFrames << " frames" << std::endl;
        }
    }

    file.close();
    std::cout << "Loaded " << frames.size() << " frames from " << filename << std::endl;
    return !frames.empty();
}

