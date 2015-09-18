#ifndef INPUTFILE_H
#define INPUTFILE_H

#include <QObject>
#include <QtCore>
#include "inputstream.h"
#include "inputchapter.h"

class InputFile : public QObject
{
    Q_OBJECT
public:
    explicit InputFile(QObject *parent = 0, QString inputFilePath = QString());

    // getters
    QString filePath() const { return _filePath; }

    bool isValid();

private:
    QString _filePath;
    QMap<int,InputStream> _streams;
    QMap<int,InputChapter> _chapters;
    QTime _duration;
    int _bitRate;

signals:
    void filePathChanged(QString filePath);

public slots:
    void setFilePath(QString &filePath);
};

#endif // INPUTFILE_H
