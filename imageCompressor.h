#ifndef IMAGECOMPRESSOR_H
#define IMAGECOMPRESSOR_H

#pragma once
#include <QObject>

class ImageCompressor : public QObject
{
    Q_OBJECT

public:
    explicit ImageCompressor(QObject *parent = nullptr);

    void compress(const QString &inputPath, int row);

signals:
    void progressUpdated(int row, int percent);
    void finished(int row, QString outputPath, double ratio);
};

#endif // IMAGECOMPRESSOR_H
