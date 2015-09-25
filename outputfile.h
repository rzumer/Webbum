#ifndef OUTPUTFILE_H
#define OUTPUTFILE_H

#include <QObject>
#include <QFileInfo>
#include <QDir>
#include <QTime>
#include <math.h>

class OutputFile : public QObject
{
    Q_OBJECT
public:
    enum outputVideoCodec {VP8, VP9};
    enum outputAudioCodec {OPUS, VORBIS};
    explicit OutputFile(QObject *parent = 0, QString outputFilePath = QString());

    // getters
    QString filePath() const { return _filePath; }
    int videoCodec() const { return _videoCodec; }
    int audioCodec() const { return _audioCodec; }
    QTime startTime() const { return _startTime; }
    QTime endTime() const { return _endTime; }
    int bitRate() const { return _bitRate; }
    int width() const { return _width; }
    int height() const { return _height; }
    int cropLeft() const { return _cropLeft; }
    int cropRight() const { return _cropRight; }
    int cropTop() const { return _cropTop; }
    int cropBottom() const { return _cropBottom; }

    static bool isValid(QString outputFilePath);
    bool isValid();
    bool isExistingFile();
    static OutputFile fromInputFile(QObject *parent, QString inputFilePath);

private:
    QString _filePath;
    int _videoCodec;
    int _audioCodec;
    QTime _startTime;
    QTime _endTime;
    int _bitRate;
    int _width;
    int _height;
    int _cropLeft;
    int _cropRight;
    int _cropTop;
    int _cropBottom;

signals:
    void outputFileChanged(QString filePath);

public slots:
    void setFilePath(QString &filePath);
    void setVideoCodec(int videoCodec) { _videoCodec = videoCodec; }
    void setAudioCodec(int audioCodec) { _audioCodec = audioCodec; }
    void setStartTime(QTime startTime) { _startTime = startTime; }
    void setEndTime(QTime endTime) { _endTime = endTime; }
    void setBitRate(double bitRate, int multiplier = 1) { _bitRate = (int)(round(bitRate)) * multiplier; }
    void setBitRateInKilobits(double bitRateInKilobits) { setBitRate(bitRateInKilobits, 1000); }
    void setBitRateForBytes(double sizeInBytes, double durationInMSecs = 0);
    void setBitRateForMegabytes(double sizeInMegabytes, double durationInMSecs = 0);
    void setWidth(int width) { _width = width; }
    void setHeight(int height) { _height = height; }
    void setCropLeft(int cropLeft) { _cropLeft = cropLeft; }
    void setCropRight(int cropRight) { _cropRight = cropRight; }
    void setCropTop(int cropTop) { _cropTop = cropTop; }
    void setCropBottom(int cropBottom) { _cropBottom = cropBottom; }
};

#endif // OUTPUTFILE_H
