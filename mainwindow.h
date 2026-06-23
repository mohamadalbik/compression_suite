#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThreadPool>
#include <QElapsedTimer>
#include "DropTableWidget.h"
#include "imageCompressor.h"
#include "VideoCodec.h"
#include <qstatusbar.h>
#include "VideoPlayer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct VideoTask {
    QString inputPath;
    QString outputPath;
    QString outputFormat;
    int quality;        // 0=Weak, 1=Standard, 2=Strong
    QString resolution; // e.g., "640x480"
    double fps;
    int targetWidth;
    int targetHeight;
    double targetFps;
    bool resizeEnabled;
    bool fpsChangeEnabled;
};

struct ImageTask {
    QString inputPath;
    QString outputPath;
    int quality;        // 0=Weak, 1=Standard, 2=Strong
    QString format;     // "jpg", "png", "myimg"
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // Video tab
    void onAddVideos();
    void onStartVideos();
    void onSelectOutputDir();
    void decodeVideoToStandardFormat(const std::vector<frameData>& encodedFrames,
                                     int fps, const std::string& outputPath,
                                     const QString& format, double quality);
    void onOpenVideo();
    void onPlayVideo();
    void onPauseVideo();
    void onStopVideo();
    void onSeekChanged(int value);
    void updateVideoDisplay(const QImage& image);
    void updateSeekProgress(int frame, int total);
    void toggleFullScreen();
    void exitFullScreen();

    // Image tab
    void onAddImages();
    void onStartImages();
    void onSelectImageOutputDir();

    // Quality mapping
    int getVideoQualityFromUI();
    int getImageQualityFromUI();
    QString getOutputFormatFromUI();
    QString getVideoOutputFormat();   // Returns "myvid", "mp4", or "avi"
    QString getImageOutputFormat();

    // Progress updates
    void updateVideoProgress(int row, int percent, const QString& status);
    void updateImageProgress(int row, int percent, const QString& status);
    void onVideoFinished(int row, bool success, double ratio, double elapsedMs, double compressedSizeMB);
    void onImageFinished(int row, bool success, double ratio, double elapsedMs);

    // Clear table
    void onClearVideos();
    void onClearImages();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    Ui::MainWindow *ui;


    // Thread management
    QThreadPool* threadPool;
    QElapsedTimer timer;
    QString outputDirectory;
    QString imageOutputDirectory;
    // Helper methods
    void addVideoRow(const QString &filePath);
    void addImageRow(const QString &filePath);
    void processVideo(int row, const VideoTask& task);
    void processImage(int row, const ImageTask& task);
    QString getResolutionString(const QString& resolution);
    double getQualityFactor(int quality);

    VideoPlayer* m_videoPlayer;
    bool m_isPlaying;
    bool m_isFullScreen;
    QRect m_normalGeometry;
    QWidget* m_videoContainer;
};

#endif