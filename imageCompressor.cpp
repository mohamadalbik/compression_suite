#include "imagecompressor.h"
#include <QThread>
#include <QFileInfo>

ImageCompressor::ImageCompressor(QObject *parent)
    : QObject(parent)
{
}

void ImageCompressor::compress(const QString &inputPath, int row)
{
    QFileInfo info(inputPath);
    qint64 originalSize = info.size();


    for (int i = 0; i <= 100; i++)
    {
        QThread::msleep(20);
        emit progressUpdated(row, i);
    }


    qint64 compressedSize = originalSize * 0.4;

    double ratio = (compressedSize * 100.0) / originalSize;

    emit finished(row, inputPath, ratio);
}