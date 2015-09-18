#ifndef OUTPUTFILE_H
#define OUTPUTFILE_H

#include <QObject>
#include <QtCore>

class OutputFile : public QObject
{
    Q_OBJECT
public:
    explicit OutputFile(QObject *parent = 0, QString outputFilePath = QString());

    // getters
    QString filePath() const { return _filePath; }

    bool isValid();

private:
    QString _filePath;

signals:
    void filePathChanged(QString filePath);

public slots:
    void setFilePath(QString &filePath);
};

#endif // OUTPUTFILE_H
