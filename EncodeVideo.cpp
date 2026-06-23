#include "VideoCodec.h"
#include "ContainerFormat.h"

void encodeVideo(const std::string videoPath, std::vector<frameData>& encodedFrames, int& fps, double quality) {


    Mat Q_Y = (cv::Mat_<float>(8, 8) <<
        16, 11, 10, 16, 24, 40, 51, 61,
        12, 12, 14, 19, 26, 58, 60, 55,
        14, 13, 16, 24, 40, 57, 69, 56,
        14, 17, 22, 29, 51, 87, 80, 62,
        18, 22, 37, 56, 68, 109, 103, 77,
        24, 35, 55, 64, 81, 104, 113, 92,
        49, 64, 78, 87, 103, 121, 120, 101,
        72, 92, 95, 98, 112, 100, 103, 99
        );

    Mat Q_C = (Mat_<float>(8, 8) <<
        17, 18, 24, 47, 99, 99, 99, 99,
        18, 21, 26, 66, 99, 99, 99, 99,
        24, 26, 56, 99, 99, 99, 99, 99,
        47, 66, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99
        );

    Q_Y *= quality;
    Q_C *= quality;
   
    std::vector<cv::Mat> frames;
    extractFrame(videoPath, frames, fps);

    std::cout << "Encoding Started....\n";


    int GOP = 15;
    MotionEstimator estimator(16, 16, false, 1);

    cv::Mat referenceY;

    for (int i = 0; i < frames.size(); i++) {
        frameData f;
        bool isIFrame = (i % GOP == 0);
        f.isIFrame = isIFrame;

        cv::Mat Y, Cr, Cb;
        BGRtoYCbCr(frames[i], Y, Cr, Cb);

        if (isIFrame) {
            f.encodedFrame = encodeImage(frames[i], Q_Y, Q_C);
            cv::Mat decodedFrame = decodeImage(f.encodedFrame, Q_Y, Q_C);
            cv::Mat decodedY, decodedCr, decodedCb;
            BGRtoYCbCr(decodedFrame, decodedY, decodedCr, decodedCb);
            referenceY = decodedY.clone();

            f.hasMotionVectors = false;
            f.refreshChroma = true;
        }
        else {
            f.hasMotionVectors = true;
            f.refreshChroma = true;  

          
            MotionField mv;
            cv::Mat residualYFloat;
            estimator.estimate(Y, referenceY, mv, residualYFloat);

            f.mv = mv;

           
            cv::Mat residualY;
            residualYFloat += 128.0f;
            residualYFloat.convertTo(residualY, CV_8U);
            f.encodedFrame.packed_y = encodeChannel(residualY, Q_Y);

           
            cv::Mat decodedResidual = decodeChannel(f.encodedFrame.packed_y, Q_Y);
            cv::Mat decodedResidualFloat;
            decodedResidual.convertTo(decodedResidualFloat, CV_32F);
            decodedResidualFloat -= 128.0f;

           
            cv::Mat predictedY;
            estimator.compensate(referenceY, mv, predictedY);

            cv::Mat predFloat;
            predictedY.convertTo(predFloat, CV_32F);
            cv::Mat reconYFloat = predFloat + decodedResidualFloat;
            reconYFloat.convertTo(referenceY, CV_8U);

           
            cv::Mat CbDS, CrDS;
            cv::resize(Cb, CbDS, cv::Size(Cb.cols / 2, Cb.rows / 2));
            cv::resize(Cr, CrDS, cv::Size(Cr.cols / 2, Cr.rows / 2));

            f.encodedFrame.packed_cb = encodeChannel(CbDS, Q_C);
            f.encodedFrame.packed_cr = encodeChannel(CrDS, Q_C);
        }

        encodedFrames.push_back(f);
    }



    std::cout << "Encoding finished\n";
}

void encodeVideoToFile(const std::string inputVideoPath, const std::string outputFilePath, double quality) {
    std::vector<frameData> encodedFrames;
    int fps;
    uint16_t width, height;

    encodeVideo(inputVideoPath, encodedFrames, fps, quality);

    if (encodedFrames.empty()) {
        std::cerr << "Encoding failed" << std::endl;
        return;
    }

    width = encodedFrames[0].encodedFrame.packed_y.width;
    height = encodedFrames[0].encodedFrame.packed_y.height;

    VideoContainer::writeToFile(outputFilePath, encodedFrames,
        static_cast<uint8_t>(fps),
        width, height, 15);

    std::cout << "Saved to: " << outputFilePath << std::endl;
}

void encodeVideoWithSettings(const std::string videoPath,
                             std::vector<frameData>& encodedFrames,
                             int& fps,
                             const EncodeSettings& settings) {

    // Step 1: Extract all frames from input video
    std::vector<cv::Mat> frames;
    extractFrame(videoPath, frames, fps);

    if (frames.empty()) {
        std::cerr << "No frames extracted from video" << std::endl;
        return;
    }

    // Step 2: Calculate frame rate conversion
    double frameStep = 1.0;
    int originalFps = fps;

    if (settings.fpsChangeEnabled && settings.targetFps > 0) {
        frameStep = static_cast<double>(originalFps) / settings.targetFps;
        fps = static_cast<int>(settings.targetFps);
    }

    // Step 3: Initialize quantization tables with quality factor
    cv::Mat Q_Y = (cv::Mat_<float>(8, 8) <<
                       16, 11, 10, 16, 24, 40, 51, 61,
                   12, 12, 14, 19, 26, 58, 60, 55,
                   14, 13, 16, 24, 40, 57, 69, 56,
                   14, 17, 22, 29, 51, 87, 80, 62,
                   18, 22, 37, 56, 68, 109, 103, 77,
                   24, 35, 55, 64, 81, 104, 113, 92,
                   49, 64, 78, 87, 103, 121, 120, 101,
                   72, 92, 95, 98, 112, 100, 103, 99);

    cv::Mat Q_C = (cv::Mat_<float>(8, 8) <<
                       17, 18, 24, 47, 99, 99, 99, 99,
                   18, 21, 26, 66, 99, 99, 99, 99,
                   24, 26, 56, 99, 99, 99, 99, 99,
                   47, 66, 99, 99, 99, 99, 99, 99,
                   99, 99, 99, 99, 99, 99, 99, 99,
                   99, 99, 99, 99, 99, 99, 99, 99,
                   99, 99, 99, 99, 99, 99, 99, 99,
                   99, 99, 99, 99, 99, 99, 99, 99);

    // Apply quality factor (lower factor = better quality, higher factor = more compression)
    Q_Y *= settings.qualityFactor;
    Q_C *= settings.qualityFactor;

    // Step 4: Determine target dimensions
    int targetWidth = settings.resizeEnabled ? settings.targetWidth : frames[0].cols;
    int targetHeight = settings.resizeEnabled ? settings.targetHeight : frames[0].rows;

    // Step 5: Initialize motion estimator and reference frame
    MotionEstimator estimator(16, 16, false, 1);
    cv::Mat referenceY;  // Will be stored at target resolution

    // Step 6: Process each frame
    for (size_t i = 0; i < frames.size(); i++) {
        // Apply frame rate conversion
        int sourceIndex;
        if (settings.fpsChangeEnabled && settings.targetFps > 0) {
            sourceIndex = static_cast<int>(i * frameStep);
            if (sourceIndex >= static_cast<int>(frames.size())) break;
        } else {
            sourceIndex = static_cast<int>(i);
        }

        cv::Mat currentFrame = frames[sourceIndex];

        // Resize current frame if needed (BOTH current and reference will be at target resolution)
        cv::Mat resizedFrame;
        if (settings.resizeEnabled && settings.targetWidth > 0 && settings.targetHeight > 0) {
            cv::resize(currentFrame, resizedFrame, cv::Size(targetWidth, targetHeight));
        } else {
            resizedFrame = currentFrame.clone();
        }

        // Convert to YCbCr color space
        cv::Mat Y, Cr, Cb;
        BGRtoYCbCr(resizedFrame, Y, Cr, Cb);

        frameData f;
        bool isIFrame = (i % 15 == 0);
        f.isIFrame = isIFrame;

        if (isIFrame) {
            // ========== I-FRAME PROCESSING ==========
            // Encode I-frame at target resolution
            f.encodedFrame = encodeImage(resizedFrame, Q_Y, Q_C);

            // Decode I-frame to use as reference for P-frames
            cv::Mat decodedFrame = decodeImage(f.encodedFrame, Q_Y, Q_C);
            cv::Mat decodedY, decodedCr, decodedCb;
            BGRtoYCbCr(decodedFrame, decodedY, decodedCr, decodedCb);
            referenceY = decodedY.clone();  // Reference now at target resolution

            f.hasMotionVectors = false;
            f.refreshChroma = true;

        } else {
            // ========== P-FRAME PROCESSING ==========
            f.hasMotionVectors = true;
            f.refreshChroma = true;

            // Motion estimation between Y channels (both at target resolution)
            MotionField mv;
            cv::Mat residualYFloat;
            estimator.estimate(Y, referenceY, mv, residualYFloat);

            f.mv = mv;

            // Encode Y residual (shift by +128 to map to 0-255)
            cv::Mat residualY;
            residualYFloat += 128.0f;
            residualYFloat.convertTo(residualY, CV_8U);
            f.encodedFrame.packed_y = encodeChannel(residualY, Q_Y);

            // Decode Y residual for next reference frame
            cv::Mat decodedResidual = decodeChannel(f.encodedFrame.packed_y, Q_Y);
            cv::Mat decodedResidualFloat;
            decodedResidual.convertTo(decodedResidualFloat, CV_32F);
            decodedResidualFloat -= 128.0f;

            // Motion compensation to reconstruct Y
            cv::Mat predictedY;
            estimator.compensate(referenceY, mv, predictedY);

            cv::Mat predFloat;
            predictedY.convertTo(predFloat, CV_32F);
            cv::Mat reconYFloat = predFloat + decodedResidualFloat;
            reconYFloat.convertTo(referenceY, CV_8U);

            // Chroma: Intra-only (downsample at target resolution)
            cv::Mat CbDS, CrDS;
            cv::resize(Cb, CbDS, cv::Size(Cb.cols / 2, Cb.rows / 2));
            cv::resize(Cr, CrDS, cv::Size(Cr.cols / 2, Cr.rows / 2));

            f.encodedFrame.packed_cb = encodeChannel(CbDS, Q_C);
            f.encodedFrame.packed_cr = encodeChannel(CrDS, Q_C);
        }

        encodedFrames.push_back(f);

        // Progress update every 50 frames
        if ((i + 1) % 50 == 0) {
            std::cout << "Encoded " << (i + 1) << "/" << frames.size() << " frames" << std::endl;
        }
    }

    std::cout << "Encoding complete: " << encodedFrames.size() << " frames, "
              << fps << " fps, " << targetWidth << "x" << targetHeight << std::endl;
}
