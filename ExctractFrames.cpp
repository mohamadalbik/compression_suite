#include "VideoCodec.h"

void extractFrame(const std::string videoPath, std::vector<cv::Mat>& frames, int& fps) {

	cv::VideoCapture cap(videoPath);

	if (!cap.isOpened()) {

		std::cout << "Error: cannot open the video\n";
		return;

	}

	fps = static_cast<int>(cap.get(cv::CAP_PROP_FPS));

	std::cout << "FPS: " << fps << std::endl;

	cv::Mat frame;
	int frameCount = 0;
	bool success = false;
	while (true) {
		success = cap.read(frame);

		if (!success) break;

		frames.push_back(frame.clone());

		frameCount++;

	}

	std::cout << "Total frames exctracted : " << frameCount << std::endl;

	cap.release();

}
