#ifndef FFMPEGCONTROLLER_H
#define FFMPEGCONTROLLER_H

#include <QObject>
#include <QtCore>

class FFMPEGController : public QObject
{
    Q_OBJECT

public:
    FFMPEGController();
    FFMPEGController(QString outputFilePath);
    void killProcess();
    void encodePass(QStringList &encodingParameters);
    void cleanTemporaryFiles();
    void setOutputFilePath(QString outputFilePath) { this->outputFilePath = outputFilePath; }
signals:
    void failed(bool crashed);
    void passFinished(int passNumber);
private slots:
    void encodePassFinished(int exitCode, QProcess::ExitStatus exitStatus);
private:
    QProcess *ffmpegProcess;
    QString outputFilePath;
};

#endif // FFMPEGCONTROLLER_H
