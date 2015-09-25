#include "outputfile.h"

OutputFile::OutputFile(QObject *parent, QString outputFilePath) : QObject(parent)
{
    _filePath = QFileInfo(outputFilePath.trimmed()).canonicalFilePath();
}

bool OutputFile::isValid()
{
    QFileInfo file(_filePath);
    QFileInfo directory(file.path());
    return !file.exists() && directory.exists() && directory.isWritable();
}

void OutputFile::setFilePath(QString &filePath)
{
    _filePath = QFileInfo(filePath.trimmed()).canonicalFilePath();
}
