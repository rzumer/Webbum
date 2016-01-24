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
    enum OutputVideoCodec {VP8, VP9};
    enum OutputAudioCodec {OPUS, VORBIS};
    explicit OutputFile(QObject *parent = 0, QString outputFilePath = QString());

    // getters
    QString filePath() const { return _filePath; }
    int videoCodec() const { return _videoCodec; }
    int audioCodec() const { return _audioCodec; }
    QTime startTime() const { return _startTime; }
    QTime endTime() const { return _endTime; }
    int bitRate() const { return _bitRate; }
    double bitRateInKilobits() const { return (double)_bitRate / 1000; }
    int crf() const { return _crf; }
    int width() const { return _width; }
    int height() const { return _height; }
    int cropLeft() const { return _cropLeft; }
    int cropRight() const { return _cropRight; }
    int cropTop() const { return _cropTop; }
    int cropBottom() const { return _cropBottom; }
    QString customFilters() const { return _customFilters; }
    QString customParameters() const { return _customParameters; }
    static bool isValid(QString outputFilePath);
    bool isValid();

private:
    QString _filePath;
    OutputVideoCodec _videoCodec;
    OutputAudioCodec _audioCodec;
    QTime _startTime;
    QTime _endTime;
    int _bitRate;
    int _crf;
    int _width;
    int _height;
    int _cropLeft;
    int _cropRight;
    int _cropTop;
    int _cropBottom;
    QString _customFilters;
    QString _customParameters;

public slots:
    void setFilePath(QString filePath);
    void setVideoCodec(OutputVideoCodec videoCodec) { _videoCodec = videoCodec; }
    void setAudioCodec(OutputAudioCodec audioCodec) { _audioCodec = audioCodec; }
    void setVideoCodec(int videoCodec) { _videoCodec = (OutputVideoCodec)videoCodec; }
    void setAudioCodec(int audioCodec) { _audioCodec = (OutputAudioCodec)audioCodec; }
    void setStartTime(QTime startTime) { _startTime = startTime; }
    void setEndTime(QTime endTime) { _endTime = endTime; }
    void setBitRate(double bitRate) { _bitRate = (int)(round(bitRate)); }
    void setBitRateInKilobits(double bitRateInKilobits) { setBitRate(bitRateInKilobits * 1000); }
    void setBitRateInKilobits(int bitRateInKilobits) { setBitRate(bitRateInKilobits * 1000); }
    void setBitRateForBytes(double sizeInBytes, double durationInMSecs = 0);
    void setBitRateForMegabytes(double sizeInMegabytes, double durationInMSecs = 0);
    void setCrf(int crf) { _crf = crf; }
    void setWidth(int width) { _width = width; }
    void setHeight(int height) { _height = height; }
    void setCropLeft(int cropLeft) { _cropLeft = cropLeft; }
    void setCropRight(int cropRight) { _cropRight = cropRight; }
    void setCropTop(int cropTop) { _cropTop = cropTop; }
    void setCropBottom(int cropBottom) { _cropBottom = cropBottom; }
    void setCustomFilters(QString customFilters) { _customFilters = customFilters; }
    void setCustomParameters(QString customParameters) { _customParameters = customParameters; }
};

#endif // OUTPUTFILE_H
