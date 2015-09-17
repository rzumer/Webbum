#include "outputfile.h"

OutputFile::OutputFile(QObject *parent, QString outputFilePath) : QObject(parent)
{
    _filePath = outputFilePath;
}

bool OutputFile::isValid()
{
    QFileInfo file(_filePath);
    QFileInfo directory(file.path());
    return !file.exists() && directory.exists() && directory.isWritable();
}

void OutputFile::filePath(QString &filePath)
{
    _filePath = QFileInfo(filePath.trimmed()).canonicalFilePath();
    emit filePathChanged(_filePath);
}
