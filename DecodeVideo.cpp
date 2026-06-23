#include "VideoCodec.h"
#include "ContainerFormat.h"

void decodeVideo(const std::string outputFileName ,const std::vector<frameData>& encodedFrames, int& fps,double quality) {


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

    MotionEstimator estimator(16, 16, false, 1);

    cv::Mat referenceY;
    cv::Mat reconstructedCb, reconstructedCr;

    int width = encodedFrames[0].encodedFrame.packed_y.width;
    int height = encodedFrames[0].encodedFrame.packed_y.height;

    cv::VideoWriter writer;
    writer.open(outputFileName, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, cv::Size(width, height));

    for (int i = 0; i < encodedFrames.size(); i++) {
        const frameData& f = encodedFrames[i];

        if (f.isIFrame) {
            cv::Mat decodedBGR = decodeImage(f.encodedFrame, Q_Y, Q_C);
            writer.write(decodedBGR);

            cv::Mat Y, Cr, Cb;
            BGRtoYCbCr(decodedBGR, Y, Cr, Cb);
            referenceY = Y.clone();
            reconstructedCb = Cb.clone();
            reconstructedCr = Cr.clone();
        }
        else {
           
            cv::Mat decodedResidual = decodeChannel(f.encodedFrame.packed_y, Q_Y);
            cv::Mat residualYFloat;
            decodedResidual.convertTo(residualYFloat, CV_32F);
            residualYFloat -= 128.0f;

            
            cv::Mat predictedY;
            estimator.compensate(referenceY, f.mv, predictedY);

            cv::Mat predFloat;
            predictedY.convertTo(predFloat, CV_32F);
            cv::Mat reconYFloat = predFloat + residualYFloat;
            reconYFloat.convertTo(referenceY, CV_8U);

            
            cv::Mat decodedCbDS = decodeChannel(f.encodedFrame.packed_cb, Q_C);
            cv::Mat decodedCrDS = decodeChannel(f.encodedFrame.packed_cr, Q_C);

           
            cv::resize(decodedCbDS, reconstructedCb, cv::Size(width, height));
            cv::resize(decodedCrDS, reconstructedCr, cv::Size(width, height));

          
            std::vector<cv::Mat> channels = { referenceY, reconstructedCr, reconstructedCb };
            cv::Mat ycrcb;
            cv::merge(channels, ycrcb);
            cv::Mat bgr;
            cv::cvtColor(ycrcb, bgr, cv::COLOR_YCrCb2BGR);
            writer.write(bgr);
        }
    }

    writer.release();
    std::cout << "Decoding finished\n";
}

void decodeVideoFromFile(const std::string inputFilePath, const std::string outputFileName, double quality) {
    std::vector<frameData> loadedFrames;
    int fps;
    int width, height;

    if (!VideoContainer::readFromFile(inputFilePath, loadedFrames, fps, width, height)) {
        std::cerr << "Failed to load file: " << inputFilePath << std::endl;
        return;
    }

    std::cout << "Loaded: " << loadedFrames.size() << " frames, "
        << (int)fps << " fps, " << width << "x" << height << std::endl;

    int fpsInt = fps;
    decodeVideo(outputFileName,loadedFrames, fpsInt, quality);
}