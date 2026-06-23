#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QCoreApplication>
#include <QProgressBar>
#include <QDir>
#include <QFileInfo>
#include <QtConcurrent/QtConcurrent>
#include <opencv2/opencv.hpp>


// Include your compression headers
#include "ImageCodec.h"
#include "VideoCodec.h"
#include "ContainerFormat.h"
#include "FileFormating.h"
#include "VideoPlayer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , threadPool(new QThreadPool(this))
{
    ui->setupUi(this);

    // Set default output directories
    outputDirectory = QDir::currentPath() + "/compressed_videos";
    imageOutputDirectory = QDir::currentPath() + "/compressed_images";
    QDir().mkpath(outputDirectory);
    QDir().mkpath(imageOutputDirectory);

    // ========== VIDEO TAB CONNECTIONS ==========
    connect(ui->btnAdd, &QPushButton::clicked, this, &MainWindow::onAddVideos);
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::onStartVideos);
    connect(ui->tableVideos, &DropTableWidget::filesDropped, this, &MainWindow::addVideoRow);

    // ========== IMAGE TAB CONNECTIONS ==========
    connect(ui->btnAddImages, &QPushButton::clicked, this, &MainWindow::onAddImages);
    connect(ui->btnStartImages, &QPushButton::clicked, this, &MainWindow::onStartImages);
    connect(ui->tableImages, &DropTableWidget::filesDropped, this, &MainWindow::addImageRow);

    // Set default radio buttons
    ui->radioStandard->setChecked(true);
    ui->radioImgStandard->setChecked(true);
    ui->radioImgDefault->setChecked(true);

    // Set thread pool max threads
    threadPool->setMaxThreadCount(QThread::idealThreadCount());

    m_videoPlayer = new VideoPlayer(this);
    m_isFullScreen = false;
    m_videoContainer = ui->videoPreviewLabel;

    // Connect video player signals
    connect(m_videoPlayer, &VideoPlayer::frameReady, this, &MainWindow::updateVideoDisplay);
    connect(m_videoPlayer, &VideoPlayer::progressUpdated, this, &MainWindow::updateSeekProgress);
    connect(m_videoPlayer, &VideoPlayer::finished, this, [=]() {
        ui->btnPlay->setEnabled(true);
        ui->btnPause->setEnabled(false);
        ui->btnStop->setEnabled(false);
        ui->seekSlider->setEnabled(true);
        ui->videoPreviewLabel->setText("Playback finished");
    });
    connect(m_videoPlayer, &VideoPlayer::error, this, [=](const QString& msg) {
        ui->videoPreviewLabel->setText("Error: " + msg);
        ui->videoPreviewLabel->setStyleSheet("color: red; background-color: black;");
    });

    // Playback control buttons
    connect(ui->btnPlay, &QPushButton::clicked, this, [=]() {
        m_videoPlayer->play();
        ui->btnPlay->setEnabled(false);
        ui->btnPause->setEnabled(true);
        ui->btnStop->setEnabled(true);
    });

    connect(ui->btnPause, &QPushButton::clicked, this, [=]() {
        m_videoPlayer->pause();
        ui->btnPlay->setEnabled(true);
        ui->btnPause->setEnabled(false);
    });

    connect(ui->btnStop, &QPushButton::clicked, this, [=]() {
        m_videoPlayer->stop();
        ui->btnPlay->setEnabled(true);
        ui->btnPause->setEnabled(false);
        ui->btnStop->setEnabled(false);
        ui->seekSlider->setValue(0);
    });

    connect(ui->seekSlider, &QSlider::sliderMoved, this, [=](int value) {
        m_videoPlayer->seekTo(value);
    });

    // Open video button
    connect(ui->btnOpenVideo, &QPushButton::clicked, this, [=]() {
        QString path = QFileDialog::getOpenFileName(this, "Open Video", "",
                                                    "Video Files (*.myvid *.mp4 *.avi *.mkv *.mov)");
        if (path.isEmpty()) return;

        if (m_videoPlayer->loadFile(path)) {
            ui->videoPreviewLabel->setText("Loaded: " + QFileInfo(path).fileName());
            ui->videoPreviewLabel->setStyleSheet("");
            ui->btnPlay->setEnabled(true);
            ui->btnStop->setEnabled(true);
            ui->seekSlider->setEnabled(true);
            ui->seekSlider->setRange(0, m_videoPlayer->getTotalFrames());

            // Update info panel
            ui->label_info_res->setText(QString("Resolution: %1x%2").arg(m_videoPlayer->getWidth()).arg(m_videoPlayer->getHeight()));
            ui->label_info_fps->setText(QString("FPS: %1").arg(m_videoPlayer->getFPS()));
            ui->label_info_duration->setText(QString("Duration: %1 sec").arg(m_videoPlayer->getTotalFrames() / m_videoPlayer->getFPS(), 0, 'f', 1));
            ui->label_info_format->setText(m_videoPlayer->getFormat());
        }
    });

    ui->videoPreviewLabel->installEventFilter(this);

    // Make sure buttons start disabled
    ui->btnPlay->setEnabled(false);
    ui->btnPause->setEnabled(false);
    ui->btnStop->setEnabled(false);
    ui->seekSlider->setEnabled(false);

}

MainWindow::~MainWindow()
{
    delete ui;
}

// ========== QUALITY MAPPING ==========

int MainWindow::getVideoQualityFromUI()
{
    if (ui->radioWeak->isChecked()) return 0;      // Weak = higher compression
    if (ui->radioStandard->isChecked()) return 1;  // Standard = balanced
    if (ui->radioStrong->isChecked()) return 2;    // Strong = lower compression
    return 1;
}

int MainWindow::getImageQualityFromUI()
{
    if (ui->radioImgWeak->isChecked()) return 0;
    if (ui->radioImgStandard->isChecked()) return 1;
    if (ui->radioImgStrong->isChecked()) return 2;
    return 1;
}

QString MainWindow::getVideoOutputFormat()
{
    if (ui->radioButtonDefault->isChecked()) return "myvid";
    if (ui->radioButtonMP4->isChecked()) return "mp4";
    if (ui->radioButtonAVI->isChecked()) return "avi";
    return "myvid";
}

QString MainWindow::getImageOutputFormat()
{
    if (ui->radioImgDefault->isChecked()) return "BIKGP";
    if (ui->radioImgPNG->isChecked()) return "png";
    if (ui->radioImgJPG->isChecked()) return "jpg";
    return "BIKGP";
}

QString MainWindow::getOutputFormatFromUI()
{
    if (ui->radioImgDefault->isChecked()) return "BIKGP";
    if (ui->radioImgPNG->isChecked()) return "png";
    if (ui->radioImgJPG->isChecked()) return "jpg";
    return "BIKGP";
}

double MainWindow::getQualityFactor(int quality)
{
    // Maps: 0=Weak (high compression), 1=Standard, 2=Strong (low compression)
    switch(quality) {
    case 0: return 0.5;   // Higher Q = more compression
    case 1: return 1.0;   // Standard
    case 2: return 2.0;   // Lower Q = less compression
    default: return 1.0;
    }
}

QString MainWindow::getResolutionString(const QString& resolution)
{
    if (resolution == "352x240") return "352x240";
    if (resolution == "352x288") return "352x288";
    if (resolution == "320x240") return "320x240";
    if (resolution == "640x360") return "640x360";
    if (resolution == "640x480") return "640x480";
    return "original";
}

// ========== VIDEO TAB METHODS ==========

void MainWindow::onAddVideos()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select Video Files",
        "",
        "Videos (*.mp4 *.avi *.mkv *.mov *.y4m *.yuv)"
        );

    for (const QString &file : files) {
        addVideoRow(file);
    }
}

void MainWindow::addVideoRow(const QString &filePath)
{
    int row = ui->tableVideos->rowCount();
    ui->tableVideos->insertRow(row);

    QFileInfo info(filePath);

    // Column 0: Filename
    ui->tableVideos->setItem(row, 0, new QTableWidgetItem(info.fileName()));
    ui->tableVideos->setItem(row, 0, new QTableWidgetItem(info.fileName()));
    ui->tableVideos->item(row, 0)->setToolTip(filePath);

    // Column 1: Resolution (to be filled by OpenCV)
    ui->tableVideos->setItem(row, 1, new QTableWidgetItem("Detecting..."));

    // Column 2: Frame rate (to be filled)
    ui->tableVideos->setItem(row, 2, new QTableWidgetItem("Detecting..."));

    // Column 3: File size
    double sizeMB = info.size() / (1024.0 * 1024.0);
    ui->tableVideos->setItem(row, 3, new QTableWidgetItem(QString::number(sizeMB, 'f', 2) + " MB"));

    // Column 4: Duration (to be filled)
    ui->tableVideos->setItem(row, 4, new QTableWidgetItem("Detecting..."));

    // Column 5: Status
    QTableWidgetItem *statusItem = new QTableWidgetItem("Queued");
    statusItem->setForeground(QBrush(Qt::gray));
    statusItem->setTextAlignment(Qt::AlignCenter);
    ui->tableVideos->setItem(row, 5, statusItem);

    // Column 6: Progress bar
    QProgressBar *progress = new QProgressBar();
    progress->setRange(0, 100);
    progress->setValue(0);
    progress->setTextVisible(false);
    ui->tableVideos->setCellWidget(row, 6, progress);

    // Column 7: Percentage
    ui->tableVideos->setItem(row, 7, new QTableWidgetItem("0%"));

    QLabel *removeLabel = new QLabel();
    removeLabel->setText("✖");
    removeLabel->setAlignment(Qt::AlignCenter);
    removeLabel->setCursor(Qt::PointingHandCursor);
    removeLabel->setStyleSheet(
        "QLabel {"
        "   color: #888888;"
        "   font-size: 14px;"
        "   font-weight: normal;"
        "   background-color: transparent;"
        "}"
        "QLabel:hover {"
        "   color: #ff4444;"
        "}"
        );

    // Install event filter to catch clicks
    removeLabel->installEventFilter(this);
    removeLabel->setProperty("row", row);
    removeLabel->setProperty("table", "video");

    ui->tableVideos->setCellWidget(row, 8, removeLabel);


    // Detect video info asynchronously
    QtConcurrent::run([this, row, filePath]() {
        cv::VideoCapture cap(filePath.toStdString());
        if (cap.isOpened()) {
            int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
            int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
            double fps = cap.get(cv::CAP_PROP_FPS);
            double totalFrames = cap.get(cv::CAP_PROP_FRAME_COUNT);
            double duration = totalFrames / fps;

            QString resolution = QString::number(width) + "x" + QString::number(height);
            QString fpsStr = QString::number(fps, 'f', 2);
            QString durationStr = QString::number(duration, 'f', 1) + "s";

            // Update UI in main thread
            QMetaObject::invokeMethod(this, [=]() {
                if (row < ui->tableVideos->rowCount()) {
                    ui->tableVideos->setItem(row, 1, new QTableWidgetItem(resolution));
                    ui->tableVideos->setItem(row, 2, new QTableWidgetItem(fpsStr));
                    ui->tableVideos->setItem(row, 4, new QTableWidgetItem(durationStr));
                }
            });
        }
        cap.release();
    });
}

void MainWindow::onStartVideos()
{
    if (ui->tableVideos->rowCount() == 0) {
        QMessageBox::information(this, "No Files", "Please add video files first.");
        return;
    }

    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory", outputDirectory);
    if (dir.isEmpty()) return;
    outputDirectory = dir;

    int quality = getVideoQualityFromUI();
    QString outputFormat = getVideoOutputFormat();

    // Get resolution settings
    QString resolutionStr = ui->comboResolution->currentText();
    bool resizeEnabled = (resolutionStr != "Original");
    int targetWidth = 0, targetHeight = 0;

    if (resizeEnabled) {
        QStringList parts = resolutionStr.split('x');
        if (parts.size() == 2) {
            targetWidth = parts[0].toInt();
            targetHeight = parts[1].toInt();
        }
    }

    // Get FPS settings
    QString fpsStr = ui->comboFPS->currentText();
    bool fpsChangeEnabled = (fpsStr != "Original");
    double targetFps = fpsStr.toDouble();

    for (int row = 0; row < ui->tableVideos->rowCount(); row++) {
        QString inputPath = ui->tableVideos->item(row, 0)->toolTip();
        if (inputPath.isEmpty()) {
            inputPath = ui->tableVideos->item(row, 0)->text();
        }

        QFileInfo info(inputPath);
        QString outputPath = outputDirectory + "/" + info.baseName() + "." + outputFormat;

        VideoTask task;
        task.inputPath = inputPath;
        task.outputPath = outputPath;
        task.quality = quality;
        task.outputFormat = outputFormat;
        task.targetWidth = targetWidth;
        task.targetHeight = targetHeight;
        task.targetFps = targetFps;
        task.resizeEnabled = resizeEnabled;
        task.fpsChangeEnabled = fpsChangeEnabled;

        QtConcurrent::run([this, row, task]() {
            processVideo(row, task);
        });
    }
}
void MainWindow::processVideo(int row, const VideoTask& task)
{
    QElapsedTimer timer;
    timer.start();

    updateVideoProgress(row, 0, "Encoding...");


    try {

        EncodeSettings settings;
        settings.qualityFactor = getQualityFactor(task.quality);
        settings.targetWidth = task.targetWidth;
        settings.targetHeight = task.targetHeight;
        settings.targetFps = task.targetFps;
        settings.resizeEnabled = task.resizeEnabled;
        settings.fpsChangeEnabled = task.fpsChangeEnabled;

        // Always encode using YOUR algorithm first
        std::vector<frameData> encodedFrames;
        int fps;

        // Pass quality factor to your encoder
        double qualityFactor = getQualityFactor(task.quality);
        encodeVideoWithSettings(task.inputPath.toStdString(), encodedFrames, fps, settings);

        QFileInfo inputInfo(task.inputPath);
        double originalSizeMB = inputInfo.size() / (1024.0 * 1024.0);
        double compressedSizeMB = 0;
        double ratio = 0;

        if (task.outputFormat == "myvid") {
            // Save as YOUR custom .myvid format
            int width = encodedFrames[0].encodedFrame.packed_y.width;
            int height = encodedFrames[0].encodedFrame.packed_y.height;

            VideoContainer::writeToFile(task.outputPath.toStdString(), encodedFrames,
                                        static_cast<uint8_t>(fps), width, height, 15);

            QFileInfo outputInfo(task.outputPath);
            compressedSizeMB = outputInfo.size() / (1024.0 * 1024.0);
            ratio = (1 - compressedSizeMB / originalSizeMB) * 100;

            updateVideoProgress(row, 100, "Done (Custom)");

        } else {
            // For MP4/AVI, we need to decode and re-encode with FFmpeg
            updateVideoProgress(row, 70, "Converting to " + task.outputFormat.toUpper() + "...");

            // Decode your format to MP4/AVI
            decodeVideoToStandardFormat(encodedFrames, fps, task.outputPath.toStdString(), task.outputFormat, qualityFactor);

            QFileInfo outputInfo(task.outputPath);
            compressedSizeMB = outputInfo.size() / (1024.0 * 1024.0);
            ratio = (1 - compressedSizeMB / originalSizeMB) * 100;

            updateVideoProgress(row, 100, "Done (" + task.outputFormat.toUpper() + ")");
        }

        double elapsed = timer.elapsed() / 1000.0;
        onVideoFinished(row, true, ratio, elapsed, compressedSizeMB);

    } catch (const std::exception& e) {
        updateVideoProgress(row, 0, "Failed: " + QString(e.what()));
        std::cout<<QString(e.what()).toStdString() << std::endl;
        onVideoFinished(row, false, 0, timer.elapsed() / 1000.0, 0);
    }
}
void MainWindow::updateVideoProgress(int row, int percent, const QString& status)
{
    QMetaObject::invokeMethod(this, [=]() {
        if (row >= ui->tableVideos->rowCount()) return;

        QProgressBar *progress = qobject_cast<QProgressBar*>(ui->tableVideos->cellWidget(row, 6));
        if (progress) progress->setValue(percent);

        QTableWidgetItem *statusItem = ui->tableVideos->item(row, 5);
        if (statusItem) {
            statusItem->setText(status);
            if (status == "Done") {
                statusItem->setForeground(QBrush(Qt::darkGreen));
            } else if (status == "Failed") {
                statusItem->setForeground(QBrush(Qt::red));
            } else {
                statusItem->setForeground(QBrush(Qt::blue));
            }
        }

        QTableWidgetItem *percentItem = ui->tableVideos->item(row, 7);
        if (percentItem) percentItem->setText(QString::number(percent) + "%");
    });
}

void MainWindow::onVideoFinished(int row, bool success, double ratio, double elapsedMs, double compressedSizeMB)
{
    QMetaObject::invokeMethod(this, [=]() {
        if (success && row < ui->tableVideos->rowCount()) {
            // Update compressed size if column exists
            if (ui->tableVideos->columnCount() > 4) {
                // You might want to add a column for compressed size
                // For now, update the status or log
            }
        }
        // Show summary message
        statusBar()->showMessage(QString("Video %1 in %2 seconds, ratio: %3%")
                                     .arg(success ? "completed" : "failed")
                                     .arg(elapsedMs, 0, 'f', 1)
                                     .arg(ratio, 0, 'f', 1), 3000);
    });
}

void MainWindow::onOpenVideo()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Video", "",
                                                    "Video Files (*.myvid *.mp4 *.avi *.mkv *.mov)");
    if (filePath.isEmpty()) return;

    if (m_videoPlayer->loadFile(filePath)) {
        ui->videoPreviewLabel->setText("Loading...");
        ui->btnPlay->setEnabled(true);
        ui->btnStop->setEnabled(true);
        ui->seekSlider->setEnabled(true);
        ui->seekSlider->setRange(0, m_videoPlayer->getTotalFrames());

        // Update info labels
        ui->label_info_res->setText(QString("Resolution: %1x%2").arg(m_videoPlayer->getWidth()).arg(m_videoPlayer->getHeight()));
        ui->label_info_fps->setText(QString("FPS: %1").arg(m_videoPlayer->getFPS()));
        ui->label_info_duration->setText(QString("Duration: %1 sec").arg(m_videoPlayer->getTotalFrames() / m_videoPlayer->getFPS()));
        ui->label_info_format->setText(filePath.endsWith(".myvid") ? "Format: Custom (.myvid)" : "Format: Standard");
    }
}

void MainWindow::onPlayVideo()
{
    m_videoPlayer->play();
    ui->btnPlay->setEnabled(false);
    ui->btnPause->setEnabled(true);
    ui->btnStop->setEnabled(true);
    m_isPlaying = true;
}

void MainWindow::onPauseVideo()
{
    m_videoPlayer->pause();
    ui->btnPlay->setEnabled(true);
    ui->btnPause->setEnabled(false);
    m_isPlaying = false;
}

void MainWindow::onStopVideo()
{
    m_videoPlayer->stop();
    ui->btnPlay->setEnabled(true);
    ui->btnPause->setEnabled(false);
    ui->btnStop->setEnabled(false);
    ui->seekSlider->setValue(0);
    m_isPlaying = false;
}

void MainWindow::onSeekChanged(int value)
{
    m_videoPlayer->seekTo(value);
}

void MainWindow::updateVideoDisplay(const QImage& image)
{
    QPixmap pixmap = QPixmap::fromImage(image);

    // Scale to fit label while preserving aspect ratio
    QPixmap scaled = pixmap.scaled(ui->videoPreviewLabel->size(),
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);

    ui->videoPreviewLabel->setPixmap(scaled);
    ui->videoPreviewLabel->setScaledContents(false);
    ui->videoPreviewLabel->setAlignment(Qt::AlignCenter);
}

void MainWindow::updateSeekProgress(int frame, int total)
{
    if (!ui->seekSlider->isSliderDown()) {
        ui->seekSlider->setValue(frame);
    }

    double seconds = static_cast<double>(frame) / m_videoPlayer->getFPS();
    double totalSeconds = static_cast<double>(total) / m_videoPlayer->getFPS();

    int currentMin = static_cast<int>(seconds) / 60;
    int currentSec = static_cast<int>(seconds) % 60;
    int totalMin = static_cast<int>(totalSeconds) / 60;
    int totalSec = static_cast<int>(totalSeconds) % 60;

    ui->timeLabel->setText(QString("%1:%2 / %3:%4")
                               .arg(currentMin, 2, 10, QChar('0'))
                               .arg(currentSec, 2, 10, QChar('0'))
                               .arg(totalMin, 2, 10, QChar('0'))
                               .arg(totalSec, 2, 10, QChar('0')));
}

// ========== IMAGE TAB METHODS ==========

void MainWindow::onAddImages()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select Image Files",
        "",
        "Images (*.png *.jpg *.jpeg *.bmp *.tiff)"
        );

    for (const QString &file : files) {
        addImageRow(file);
    }
}

void MainWindow::addImageRow(const QString &filePath)
{
    int row = ui->tableImages->rowCount();
    ui->tableImages->insertRow(row);

    QFileInfo info(filePath);

    // Column 0: Filename
    ui->tableImages->setItem(row, 0, new QTableWidgetItem(info.fileName()));
    ui->tableImages->item(row, 0)->setToolTip(filePath);

    // Column 1: Size
    double sizeKB = info.size() / 1024.0;
    ui->tableImages->setItem(row, 1, new QTableWidgetItem(QString::number(sizeKB, 'f', 1) + " KB"));

    // Column 2: Status
    QTableWidgetItem *statusItem = new QTableWidgetItem("Queued");
    statusItem->setForeground(QBrush(Qt::gray));
    statusItem->setTextAlignment(Qt::AlignCenter);
    ui->tableImages->setItem(row, 2, statusItem);

    // Column 3: Progress bar
    QProgressBar *progress = new QProgressBar();
    progress->setRange(0, 100);
    progress->setValue(0);
    progress->setTextVisible(false);
    ui->tableImages->setCellWidget(row, 3, progress);

    // Column 4: Compressed size (placeholder)
    ui->tableImages->setItem(row, 4, new QTableWidgetItem("Pending"));

    // Column 5: Percentage
    ui->tableImages->setItem(row, 5, new QTableWidgetItem("0%"));

    QLabel *removeLabel = new QLabel();
    removeLabel->setText("✖");
    removeLabel->setAlignment(Qt::AlignCenter);
    removeLabel->setCursor(Qt::PointingHandCursor);
    removeLabel->setStyleSheet(
        "QLabel {"
        "   color: #888888;"
        "   font-size: 14px;"
        "   font-weight: normal;"
        "   background-color: transparent;"
        "}"
        "QLabel:hover {"
        "   color: #ff4444;"
        "}"
        );

    // Install event filter to catch clicks
    removeLabel->installEventFilter(this);
    removeLabel->setProperty("row", row);
    removeLabel->setProperty("table", "image");

    ui->tableImages->setCellWidget(row, 6, removeLabel);

}

void MainWindow::onStartImages()
{
    if (ui->tableImages->rowCount() == 0) {
        QMessageBox::information(this, "No Files", "Please add image files first.");
        return;
    }

    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory", imageOutputDirectory);
    if (dir.isEmpty()) return;
    imageOutputDirectory = dir;

    int quality = getImageQualityFromUI();
    QString outputFormat = getImageOutputFormat();

    for (int row = 0; row < ui->tableImages->rowCount(); row++) {
        QString inputPath = ui->tableImages->item(row, 0)->toolTip();
        if (inputPath.isEmpty()) {
            inputPath = ui->tableImages->item(row, 0)->text();
        }

        QFileInfo info(inputPath);
        QString outputPath = imageOutputDirectory + "/" + info.baseName() + "." + outputFormat;

        ImageTask task;
        task.inputPath = inputPath;
        task.outputPath = outputPath;
        task.quality = quality;
        task.format = outputFormat;

        QtConcurrent::run([this, row, task]() {
            processImage(row, task);
        });
    }
}

void MainWindow::processImage(int row, const ImageTask& task)
{
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

    // Scale based on quality (higher quality factor = lower quantization)
    double qualityFactor = getQualityFactor(task.quality);
    Q_Y *= qualityFactor;
    Q_C *= qualityFactor;


    QElapsedTimer timer;
    timer.start();

    updateImageProgress(row, 0, "Processing...");

    try {
        cv::Mat img = cv::imread(task.inputPath.toStdString());
        if (img.empty()) {
            throw std::runtime_error("Cannot load image");
        }



        if (task.format == "BIKGP") {
            // Use YOUR custom format
            EncodedImg encoded = encodeImage(img, Q_Y, Q_C);
            std::string filename = task.outputPath.toStdString();
            writeImg(filename, encoded.packed_y, encoded.packed_cr, encoded.packed_cb);

        } else if (task.format == "png") {
            // Standard PNG (lossless)
            EncodedImg encoded = encodeImage(img, Q_Y, Q_C);
            cv::Mat decoded = decodeImage(encoded, Q_Y, Q_C);
            cv::imwrite(task.outputPath.toStdString(), img);

        } else { // jpg
            // Standard JPEG
            // Standard PNG (lossless)
            EncodedImg encoded = encodeImage(img, Q_Y, Q_C);
            cv::Mat decoded = decodeImage(encoded, Q_Y, Q_C);
            cv::imwrite(task.outputPath.toStdString(), img);
    }

        // Calculate metrics
        QFileInfo inputInfo(task.inputPath);
        QFileInfo outputInfo(task.outputPath);
        double originalSizeKB = inputInfo.size() / 1024.0;
        double compressedSizeKB = outputInfo.size() / 1024.0;
        double ratio = (1 - compressedSizeKB / originalSizeKB) * 100;

        double elapsed = timer.elapsed() / 1000.0;

        // Update UI with results
        QMetaObject::invokeMethod(this, [=]() {
            if (row < ui->tableImages->rowCount()) {
                ui->tableImages->setItem(row, 4,
                                         new QTableWidgetItem(QString::number(compressedSizeKB, 'f', 1) + " KB"));
                ui->tableImages->setItem(row, 5,
                                         new QTableWidgetItem(QString::number(ratio, 'f', 1) + "%"));
            }
        });

        updateImageProgress(row, 100, "Done");
        onImageFinished(row, true, ratio, elapsed);

    } catch (const std::exception& e) {
        updateImageProgress(row, 0, "Failed: " + QString(e.what()));
        onImageFinished(row, false, 0, timer.elapsed() / 1000.0);
    }
}
void MainWindow::updateImageProgress(int row, int percent, const QString& status)
{
    QMetaObject::invokeMethod(this, [=]() {
        if (row >= ui->tableImages->rowCount()) return;

        QProgressBar *progress = qobject_cast<QProgressBar*>(ui->tableImages->cellWidget(row, 3));
        if (progress) progress->setValue(percent);

        QTableWidgetItem *statusItem = ui->tableImages->item(row, 2);
        if (statusItem) {
            statusItem->setText(status);
            if (status == "Done") {
                statusItem->setForeground(QBrush(Qt::darkGreen));
            } else if (status == "Failed") {
                statusItem->setForeground(QBrush(Qt::red));
            } else {
                statusItem->setForeground(QBrush(Qt::blue));
            }
        }

        QTableWidgetItem *percentItem = ui->tableImages->item(row, 5);
        if (percentItem && status != "Done") {
            percentItem->setText(QString::number(percent) + "%");
        }
    });
}

void MainWindow::onImageFinished(int row, bool success, double ratio, double elapsedMs)
{
    QMetaObject::invokeMethod(this, [=]() {
        this->statusBar()->showMessage(QString("Image %1 in %2 seconds, ratio: %3%")
                                       .arg(success ? "completed" : "failed")
                                       .arg(elapsedMs, 0, 'f', 1)
                                       .arg(ratio, 0, 'f', 1), 3000);
    });
}

void MainWindow::onSelectOutputDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory", outputDirectory);
    if (!dir.isEmpty()) {
        outputDirectory = dir;
        this->statusBar()->showMessage("Output directory: " + outputDirectory, 2000);
    }
}

void MainWindow::decodeVideoToStandardFormat(const std::vector<frameData>& encodedFrames,
                                             int fps, const std::string& outputPath,
                                             const QString& format, double quality){
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

    int fourcc;
    if (format == "mp4") {
        fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');  // H.264
    } else {  // avi
        fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');  // MJPEG for AVI
    }

    cv::VideoWriter writer;
    writer.open(outputPath, fourcc, fps, cv::Size(width, height));

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

void MainWindow::onSelectImageOutputDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory", imageOutputDirectory);
    if (!dir.isEmpty()) {
        imageOutputDirectory = dir;
        this->statusBar()->showMessage("Image output directory: " + imageOutputDirectory, 2000);
    }
}

void MainWindow::onClearVideos()
{
    ui->tableVideos->setRowCount(0);
}

void MainWindow::onClearImages()
{
    ui->tableImages->setRowCount(0);
}

void MainWindow::toggleFullScreen()
{
    if (!m_isFullScreen) {
        // Enter full screen
        m_normalGeometry = ui->videoPreviewLabel->geometry();
        m_isFullScreen = true;

        // Create temporary full screen window
        QWidget* fullScreenWidget = new QWidget();
        fullScreenWidget->setWindowFlags(Qt::Window);
        fullScreenWidget->setStyleSheet("background-color: black;");

        QVBoxLayout* layout = new QVBoxLayout(fullScreenWidget);
        QLabel* fullScreenLabel = new QLabel();
        fullScreenLabel->setAlignment(Qt::AlignCenter);
        fullScreenLabel->setScaledContents(true);

        // Copy current pixmap to full screen
        QPixmap currentPixmap = ui->videoPreviewLabel->pixmap();
        if (!currentPixmap.isNull()) {
            fullScreenLabel->setPixmap(currentPixmap.scaled(
                QApplication::primaryScreen()->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation));
        }

        layout->addWidget(fullScreenLabel);

        // Add exit button
        QPushButton* exitBtn = new QPushButton("Exit Full Screen (ESC)");
        exitBtn->setStyleSheet("background-color: #333; color: white; padding: 10px;");
        layout->addWidget(exitBtn, 0, Qt::AlignCenter);

        connect(exitBtn, &QPushButton::clicked, [fullScreenWidget]() {
            fullScreenWidget->close();
        });

        fullScreenWidget->showFullScreen();

        // Connect to update frames in full screen
        connect(m_videoPlayer, &VideoPlayer::frameReady, this, [fullScreenLabel](const QImage& image) {
            QPixmap pixmap = QPixmap::fromImage(image);
            fullScreenLabel->setPixmap(pixmap.scaled(
                QApplication::primaryScreen()->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation));
        });

        connect(fullScreenWidget, &QWidget::destroyed, this, &MainWindow::exitFullScreen);


    } else {
        exitFullScreen();
    }
}

void MainWindow::exitFullScreen()
{
    m_isFullScreen = false;
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{

    if (event->type() == QEvent::MouseButtonPress) {
        QLabel* label = qobject_cast<QLabel*>(obj);
        if (label && label->cursor() == Qt::PointingHandCursor) {
            int row = label->property("row").toInt();
            QString table = label->property("table").toString();

            if (table == "video") {
                ui->tableVideos->removeRow(row);
            } else if (table == "image") {
                ui->tableImages->removeRow(row);
            }
            return true;
        }
    }


    if (obj == ui->videoPreviewLabel && event->type() == QEvent::MouseButtonDblClick) {
        toggleFullScreen();
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

