#include "outputfile.h"

OutputFile::OutputFile(QObject *parent, QString outputFilePath) : QObject(parent)
{
    _cropLeft = 0;
    _cropRight = 0;
    _cropTop = 0;
    _cropBottom = 0;

    QFileInfo file(outputFilePath.trimmed());
    if(file.exists())
        setFilePath(file.canonicalPath() + "/" + file.completeBaseName() + ".webm");
    else
        setFilePath(file.filePath());
}

bool OutputFile::isValid(QString outputFilePath)
{
    QFileInfo file(outputFilePath);
    QFileInfo directory(file.path());
    return !file.baseName().isEmpty() && !file.exists() && directory.exists() && directory.isWritable();
}

bool OutputFile::isValid()
{
    return isValid(_filePath);
}

void OutputFile::setFilePath(QString filePath)
{
    QFileInfo file(filePath.trimmed());
    _filePath = file.filePath();

    while(file.exists())
    {
        _filePath = file.canonicalPath() + "/" + file.completeBaseName() + "_out.webm";
        file = QFileInfo(_filePath);
    }

    emit outputFileChanged(_filePath);
}

void OutputFile::setBitRateForBytes(double sizeInBytes, double durationInMSecs)
{
    if(durationInMSecs > 0)
        setBitRate((sizeInBytes * 8) / durationInMSecs / 1000);
    else
        setBitRate((sizeInBytes * 8) / _startTime.msecsTo(_endTime) / 1000);
}

void OutputFile::setBitRateForMegabytes(double sizeInMegabytes, double durationInMSecs)
{
    setBitRateForBytes(sizeInMegabytes * 1024 * 1024, durationInMSecs);
}
