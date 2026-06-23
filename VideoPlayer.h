#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <opencv2/opencv.hpp>
#include "VideoCodec.h"
#include "ContainerFormat.h"
#include "MotionEstimation.h"

class VideoPlayer : public QObject
{
    Q_OBJECT

public:
    explicit VideoPlayer(QObject *parent = nullptr);
    ~VideoPlayer();

    bool loadFile(const QString& filePath);
    void play();
    void pause();
    void stop();
    void seekTo(int frameNumber);

    int getTotalFrames() const { return m_totalFrames; }
    int getCurrentFrame() const { return m_currentFrame; }
    double getFPS() const { return m_fps; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    QString getFormat() const { return m_isMyVid ? "Custom (.myvid)" : "Standard"; }

signals:
    void frameReady(const QImage& image);
    void progressUpdated(int frame, int total);
    void finished();
    void error(const QString& message);

private slots:
    void decodeNextFrame();

private:
    bool loadMyVid(const QString& filePath);
    bool loadStandardVideo(const QString& filePath);
    void resetDecoderState();
    cv::Mat decodePFrame(const frameData& f);

    // MYVID specific
    std::vector<frameData> m_frames;
    cv::Mat m_referenceY;
    cv::Mat m_referenceCb;
    cv::Mat m_referenceCr;
    cv::Mat m_Q_Y;
    cv::Mat m_Q_C;
    MotionEstimator* m_estimator;

    // Standard video
    cv::VideoCapture m_cap;

    // Common
    bool m_isMyVid;
    int m_currentFrame;
    int m_totalFrames;
    double m_fps;
    int m_width;
    int m_height;
    bool m_playing;
    QTimer* m_timer;
};

#endif