#include "ffmpegcontroller.h"

FFMPEGController::FFMPEGController()
{
    ffmpegProcess = new QProcess(this);
}

FFMPEGController::FFMPEGController(QString outputFilePath)
{
    FFMPEGController();
    this->outputFilePath = outputFilePath;
}

void FFMPEGController::encodePass(QStringList &encodingParameters)
{
    ffmpegProcess = new QProcess(this);
    connect(ffmpegProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(encodePassFinished(int,QProcess::ExitStatus)));
    connect(ffmpegProcess, SIGNAL(finished(int,QProcess::ExitStatus)), ffmpegProcess, SLOT(deleteLater()));

    ffmpegProcess->start("ffmpeg",encodingParameters);
}

void FFMPEGController::encodePassFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(exitCode != 0 || exitStatus == QProcess::CrashExit)
    {
        if(!outputFilePath.isEmpty())
        {
            // recover from crash
            cleanTemporaryFiles();

            // delete incomplete encode
            QFile outputFile(outputFilePath);
            if(outputFile.exists())
                outputFile.remove();
        }

        // Cancelling an encode causes an exit status of 1 (QProcess::CrashExit).
        // Therefore, a crash is considered to occur when the exit status is neither 0 nor 1.
        emit failed(exitStatus != QProcess::CrashExit);
    }
    else
    {
        int passNumberIndex = ffmpegProcess->arguments().indexOf("-pass") + 1;
        if(QString(ffmpegProcess->arguments().at(passNumberIndex)).toInt() == 1)
        {
            // pass 1 finished
            emit passFinished(1);
        }
        else
        {
            if(!outputFilePath.isEmpty())
            {
                // last pass finished
                cleanTemporaryFiles();
            }

            emit passFinished(2);
        }
    }
}

void FFMPEGController::killProcess()
{
    if(ffmpegProcess && ffmpegProcess->state() == QProcess::Running)
    {
        ffmpegProcess->kill();
        ffmpegProcess->waitForFinished();
    }
}

void FFMPEGController::cleanTemporaryFiles()
{
    QDir outputDirectory = QFileInfo(outputFilePath).dir();
    QDir tempDirectory = outputDirectory.canonicalPath() + "/temp";
    QFile logFile("ffmpeg2pass-0.log");

    if(tempDirectory.exists())
        tempDirectory.removeRecursively();
    if(logFile.exists())
        logFile.remove();
}
