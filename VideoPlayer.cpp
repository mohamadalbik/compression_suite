#include "VideoPlayer.h"
#include <QDebug>
#include <QThread>

VideoPlayer::VideoPlayer(QObject *parent)
    : QObject(parent)
    , m_estimator(nullptr)
    , m_isMyVid(false)
    , m_currentFrame(0)
    , m_totalFrames(0)
    , m_fps(30.0)
    , m_width(0)
    , m_height(0)
    , m_playing(false)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &VideoPlayer::decodeNextFrame);

    // Initialize quantization tables once
    m_Q_Y = (cv::Mat_<float>(8, 8) <<
                 16, 11, 10, 16, 24, 40, 51, 61,
             12, 12, 14, 19, 26, 58, 60, 55,
             14, 13, 16, 24, 40, 57, 69, 56,
             14, 17, 22, 29, 51, 87, 80, 62,
             18, 22, 37, 56, 68, 109, 103, 77,
             24, 35, 55, 64, 81, 104, 113, 92,
             49, 64, 78, 87, 103, 121, 120, 101,
             72, 92, 95, 98, 112, 100, 103, 99);

    m_Q_C = (cv::Mat_<float>(8, 8) <<
                 17, 18, 24, 47, 99, 99, 99, 99,
             18, 21, 26, 66, 99, 99, 99, 99,
             24, 26, 56, 99, 99, 99, 99, 99,
             47, 66, 99, 99, 99, 99, 99, 99,
             99, 99, 99, 99, 99, 99, 99, 99,
             99, 99, 99, 99, 99, 99, 99, 99,
             99, 99, 99, 99, 99, 99, 99, 99,
             99, 99, 99, 99, 99, 99, 99, 99);
}

VideoPlayer::~VideoPlayer()
{
    stop();
    if (m_estimator) {
        delete m_estimator;
        m_estimator = nullptr;
    }
}

void VideoPlayer::resetDecoderState()
{
    m_referenceY.release();
    m_referenceCb.release();
    m_referenceCr.release();
    m_currentFrame = 0;

    if (m_estimator) {
        delete m_estimator;
    }
    m_estimator = new MotionEstimator(16, 16, false, 1);
}

bool VideoPlayer::loadMyVid(const QString& filePath)
{
    qDebug() << "Loading MYVID file:" << filePath;

    m_frames.clear();
    resetDecoderState();

    int fps, width, height;

    if (!VideoContainer::readFromFile(filePath.toStdString(), m_frames, fps, width, height)) {
        emit error("Failed to load .myvid file");
        return false;
    }

    if (m_frames.empty()) {
        emit error("No frames in file");
        return false;
    }

    m_isMyVid = true;
    m_totalFrames = static_cast<int>(m_frames.size());
    m_fps = fps;
    m_width = width;
    m_height = height;

    qDebug() << "Loaded" << m_totalFrames << "frames, " << m_fps << "fps, " << m_width << "x" << m_height;

    emit progressUpdated(0, m_totalFrames);
    return true;
}

bool VideoPlayer::loadStandardVideo(const QString& filePath)
{
    m_cap.open(filePath.toStdString());
    if (!m_cap.isOpened()) {
        emit error("Failed to open standard video");
        return false;
    }

    m_isMyVid = false;
    m_totalFrames = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_COUNT));
    m_fps = m_cap.get(cv::CAP_PROP_FPS);
    m_width = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_WIDTH));
    m_height = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    m_currentFrame = 0;

    emit progressUpdated(0, m_totalFrames);
    return true;
}

bool VideoPlayer::loadFile(const QString& filePath)
{
    stop();

    if (filePath.endsWith(".myvid", Qt::CaseInsensitive)) {
        return loadMyVid(filePath);
    } else {
        return loadStandardVideo(filePath);
    }
}

void VideoPlayer::play()
{
    if (m_playing) return;
    if (m_currentFrame >= m_totalFrames) {
        stop();
        return;
    }
    m_playing = true;
    int interval = static_cast<int>(1000.0 / m_fps);
    m_timer->start(interval);
}

void VideoPlayer::pause()
{
    m_playing = false;
    m_timer->stop();
}

void VideoPlayer::stop()
{
    pause();
    m_currentFrame = 0;

    if (m_isMyVid) {
        resetDecoderState();
    } else {
        m_cap.set(cv::CAP_PROP_POS_FRAMES, 0);
    }

    emit progressUpdated(0, m_totalFrames);
}

void VideoPlayer::seekTo(int frameNumber)
{
    if (frameNumber < 0) frameNumber = 0;
    if (frameNumber >= m_totalFrames) frameNumber = m_totalFrames - 1;

    m_currentFrame = frameNumber;

    if (m_isMyVid) {
        // Reset and decode forward to target frame
        resetDecoderState();
        m_currentFrame = 0;

        // Fast-forward to target (update UI after seeking)
        emit progressUpdated(m_currentFrame, m_totalFrames);
    } else {
        m_cap.set(cv::CAP_PROP_POS_FRAMES, frameNumber);
    }
}

cv::Mat VideoPlayer::decodePFrame(const frameData& f)
{
    // 1. Decode Y residual
    cv::Mat decodedResidual = decodeChannel(f.encodedFrame.packed_y, m_Q_Y);
    cv::Mat residualYFloat;
    decodedResidual.convertTo(residualYFloat, CV_32F);
    residualYFloat -= 128.0f;


    cv::Mat predictedY;
    m_estimator->compensate(m_referenceY, f.mv, predictedY);

    cv::Mat predFloat;
    predictedY.convertTo(predFloat, CV_32F);
    cv::Mat reconYFloat = predFloat + residualYFloat;
    cv::Mat reconstructedY;
    reconYFloat.convertTo(reconstructedY, CV_8U);


    cv::Mat decodedCbDS = decodeChannel(f.encodedFrame.packed_cb, m_Q_C);
    cv::Mat decodedCrDS = decodeChannel(f.encodedFrame.packed_cr, m_Q_C);


    cv::Mat reconstructedCb, reconstructedCr;
    cv::resize(decodedCbDS, reconstructedCb, cv::Size(m_width, m_height));
    cv::resize(decodedCrDS, reconstructedCr, cv::Size(m_width, m_height));


    m_referenceY = reconstructedY.clone();

    std::vector<cv::Mat> channels = { reconstructedY, reconstructedCr, reconstructedCb };
    cv::Mat ycrcb;
    cv::merge(channels, ycrcb);
    cv::Mat bgr;
    cv::cvtColor(ycrcb, bgr, cv::COLOR_YCrCb2BGR);

    return bgr;
}

void VideoPlayer::decodeNextFrame()
{
    if (!m_playing && m_currentFrame > 0) return;

    if (m_currentFrame >= m_totalFrames) {
        stop();
        emit finished();
        return;
    }

    cv::Mat frame;

    if (m_isMyVid) {
        if (m_currentFrame < 0 || m_currentFrame >= static_cast<int>(m_frames.size())) {
            stop();
            return;
        }

        const frameData& f = m_frames[m_currentFrame];

        try {
            if (f.isIFrame) {
                // I-frame: decode directly
                frame = decodeImage(f.encodedFrame, m_Q_Y, m_Q_C);

                // Store reference for P-frames
                cv::Mat Y, Cr, Cb;
                BGRtoYCbCr(frame, Y, Cr, Cb);
                m_referenceY = Y.clone();
                m_referenceCb = Cb.clone();
                m_referenceCr = Cr.clone();
            } else {
                // P-frame: use motion compensation
                frame = decodePFrame(f);
            }
        } catch (const std::exception& e) {
            emit error(QString("Decode error: %1").arg(e.what()));
            stop();
            return;
        }
    } else {
        m_cap >> frame;
        if (frame.empty()) {
            stop();
            return;
        }
    }

    if (!frame.empty()) {
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        QImage qimg(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        emit frameReady(qimg.copy());
    }

    m_currentFrame++;
    emit progressUpdated(m_currentFrame, m_totalFrames);
}