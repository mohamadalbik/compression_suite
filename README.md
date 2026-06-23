# Image & Video Compression System
A fully working image and video compression application built with C++ and Qt. The application provides a graphical user interface for compressing images and videos with customizable quality and resolution settings.
## Features 
- Image compression with adjustable quality levels
- Video compression with custom resolution and bitrate
- Clean and intuitive GUI built with Qt
- Real-time preview of compression results
- Support for multiple image and video formats
## Screenshots
![Main Window](compression_suite/.pics/photo_1_2026-06-23_05-14-59.jpg)
## Technologies
- C++
- Qt Framework
- OpenCV
- MinGW Compiler
## How to Build
### Prerequisites Before building the project, make sure you have the following installed:
- ***Qt*** (5.x or 6.x) — [Download here](https://www.qt.io/download)
- ***OpenCV*** Must be built from source using CMake with the MinGW compiler
- CMake (3.10 or higher) — [Download here](https://cmake.org/download)
- MinGW compiler
### Building OpenCV from Source
If you don't have OpenCV pre-built with MinGW, follow these steps:
1. Download the OpenCV source from [opencv.org(https://opencv.org/releases/) 
2. Open CMake GUI 
3. Set the source path to the OpenCV source folder 
4. Set the build path to opencv/build 
5. ClickConfigure and select MinGW Makefiles
6. Click Generate
7. Open terminal in the build folder and run:
- mingw32-make -j4
- mingw32-make install
### Building This Project 
1. **Clone this repository:** git clone https://github.com/your-username/image-video-compressor.git
2. ***Open `CMakeLists.txt`*** and update the file paths to match your local system:
- Change the OpenCV build path to your OpenCV build directory
- Change the Qt path to your Qt installation directory
Example:
- set(OpenCV_DIR "C:/opencv/build")
- set(Qt5_DIR "C:/Qt/5.15.2/mingw81_64/lib/cmake/Qt5")
3. **Create a build folder and run CMake:**
mkdir build cd build cmake -G "MinGW Makefiles" ..
4. **Build the project:**
mingw32-make -j4
## Notes 
- This project was built and tested on Windows using the MinGW compiler
- Make sure all dependency paths in CMakeLists.txt are correct for your system before building
- If you encounter linking errors, double-check that OpenCV and Qt paths point to the correct directories
## About 
This Project Built as a personal project to explore multimedia processing, image and video compression algorithms, and desktop application development with Qt.
