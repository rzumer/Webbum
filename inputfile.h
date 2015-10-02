#ifndef INPUTFILE_H
#define INPUTFILE_H

#include <QObject>
#include <QMap>
#include <QTime>
#include <QFileInfo>
#include <math.h>
extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
}
#include "inputstream.h"
#include "inputchapter.h"

class InputFile : public QObject
{
    Q_OBJECT
public:
    explicit InputFile(QObject *parent = 0, QString inputFilePath = QString());

    // getters
    QString filePath() const { return _filePath; }
    int streamCount() const { return _streams.size(); }
    InputStream stream(int index) const { return _streams.at(index); }
    int chapterCount() const { return _chapters.size(); }
    InputChapter chapter(int index) const { return _chapters.at(index); }
    QTime duration() const { return _duration; }
    int bitRate() const { return _bitRate; }
    int bitRateInKilobits() const { return (int)(round(_bitRate / 1000)); }
    double fileSize(double durationInMSecs = 0) const;
    double fileSize(QTime duration) const;
    double fileSizeInMegabytes(double durationInMSecs = 0) const;
    double fileSizeInMegabytes(QTime duration) const;
    int width() const { return _width; }
    int height() const { return _height; }

    static bool isValid(QString inputFilePath);
    bool isValid();
    void dumpStreamInformation();

private:
    QString _filePath;
    QList<InputStream> _streams;
    QList<InputChapter> _chapters;
    QTime _duration;
    int _bitRate;
    int _width;
    int _height;

signals:
    void inputFileChanged(QString filePath);

public slots:
};

#endif // INPUTFILE_H
