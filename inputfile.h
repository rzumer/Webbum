#ifndef INPUTFILE_H
#define INPUTFILE_H

#include <QObject>
#include <QtCore>

class InputFile : public QObject
{
    Q_OBJECT
public:
    explicit InputFile(QObject *parent = 0,QString inputFilePath = QString());

    // getters
    QString filePath() const { return _filePath; }

    bool isValid();

private:
    QString _filePath;

signals:

public slots:
    void filePath(QString &filePath);
};

#endif // INPUTFILE_H
