#ifndef INPUTFILE_H
#define INPUTFILE_H

#include <QObject>
#include <QMap>
#include <QTime>
#include <QFileInfo>
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

    static bool isValid(QString inputFilePath);
    bool isValid();
    void dumpStreamInformation();

private:
    QString _filePath;
    QMap<int,InputStream> _streams;
    QMap<int,InputChapter> _chapters;
    QTime _duration;
    int _bitRate;

signals:
    void inputFileChanged(QString filePath);

public slots:
};

#endif // INPUTFILE_H
