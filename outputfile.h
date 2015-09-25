#ifndef OUTPUTFILE_H
#define OUTPUTFILE_H

#include <QObject>
#include <QtCore>
#include "inputstream.h"

class OutputFile : public QObject
{
    Q_OBJECT
public:
    enum outputVideoCodec {VP8, VP9};
    enum outputAudioCodec {OPUS, VORBIS};
    explicit OutputFile(QObject *parent = 0, QString outputFilePath = QString());

    // getters
    QString filePath() const { return _filePath; }
    // is getting the streams necessary? can IDs be returned? do they have to be member variables?
    InputStream videoStream() const { return _videoStream; }
    InputStream audioStream() const { return _audioStream; }
    InputStream subtitleStream() const { return _subtitleStream; }
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

    bool isValid();

private:
    QString _filePath;
    InputStream _videoStream;
    InputStream _audioStream;
    InputStream _subtitleStream;
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

public slots:
    void setFilePath(QString &filePath) { _filePath = QFileInfo(filePath.trimmed()).canonicalFilePath(); }
    void setVideoStream(InputStream &videoStream) { _videoStream = videoStream; }
    void setAudioStream(InputStream &audioStream) { _audioStream = audioStream; }
    void setSubtitleStream(InputStream &subtitleStream) { _subtitleStream = subtitleStream; }
    void setVideoCodec(int videoCodec) { _videoCodec = videoCodec; }
    void setAudioCodec(int audioCodec) { _audioCodec = audioCodec; }
    void setStartTime(QTime &startTime) { _startTime = startTime; }
    void setEndTime(QTime &endTime) { _endTime = endTime; }
    void setBitRate(int bitRate) { _bitRate = bitRate; }
    void setWidth(int width) { _width = width; }
    void setHeight(int height) { _height = height; }
    void setCropLeft(int cropLeft) { _cropLeft = cropLeft; }
    void setCropRight(int cropRight) { _cropRight = cropRight; }
    void setCropTop(int cropTop) { _cropTop = cropTop; }
    void setCropBottom(int cropBottom) { _cropBottom = cropBottom; }
};

#endif // OUTPUTFILE_H
