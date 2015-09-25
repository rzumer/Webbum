#include "outputfile.h"

OutputFile::OutputFile(QObject *parent, QString outputFilePath) : QObject(parent)
{
    QFileInfo file(outputFilePath.trimmed());

    _filePath = file.canonicalFilePath();

    while(file.exists())
    {
        _filePath = file.dir().canonicalPath() + "/" + file.completeBaseName() + "_out.webm";
        file = QFileInfo(_filePath);
    }

    emit outputFileChanged(_filePath);
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

bool OutputFile::isExistingFile()
{
    QFileInfo file(_filePath);
    return file.exists() && !file.isDir();
}

void OutputFile::setFilePath(QString &filePath)
{
    _filePath = QFileInfo(filePath.trimmed()).canonicalFilePath();
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
