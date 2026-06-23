cmake_minimum_required(VERSION 3.16)

project(CompressionSuite VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/opencv-4.13.0/build-mingw/install")

find_package(OpenCV REQUIRED)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        BitPacking.h
        BGRtoYCbCr.cpp
        8x8Block.cpp
        compressImg.cpp
        compressionRatio.cpp
        ComputeResidual.cpp
        ContainerFormat.cpp
        ContainerFormat.h
        DCT2.cpp
        DecodeChannel.cpp
        DecodeImage.cpp
        DecodeVideo.cpp
        Dequantization.cpp
        DownSampling420.cpp
        encodeChannel.cpp
        EncodeVideo.cpp
        encodeImage.cpp
        ExctractFrames.cpp
        FileFormating.h
        Huffman.h
        HuffmanEncode.cpp
        IDCT1D.cpp
        IDCT2D.cpp
        ImageCodec.h
        JPEGModules.h
        MotionEstimation.cpp
        MotionEstimation.h
        PackBits.cpp
        PSNR.cpp
        quantize.cpp
        QuatizationLoss.cpp
        readFreqTable.cpp
        readImg.cpp
        reconstructed.cpp
        ReconstructResidual.cpp
        RLE.cpp
        RLE.h
        ssim.cpp
        VideoCodec.h
        writeFreqTable.cpp
        WriteImg.cpp
        zigzag8x8.cpp
        DropTableWidget.cpp
        DropTableWidget.h
)

add_executable(CompressionSuite ${PROJECT_SOURCES}
    recources.qrc
    DropTableWidget.h
    DropTableWidget.cpp
    imageCompressor.h
    imageCompressor.cpp
    VideoPlayer.h
    VideoPlayer.cpp)

target_link_libraries(CompressionSuite PRIVATE ${OpenCV_LIBS})
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET CompressionSuite APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")

target_link_libraries(CompressionSuite PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CompressionSuite)
endif()
set_target_properties(CompressionSuite PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS CompressionSuite
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(CompressionSuite)
endif()
