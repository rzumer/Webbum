#include "inputfile.h"

InputFile::InputFile(QObject *parent, QString inputFilePath) : QObject(parent)
{
    _filePath = inputFilePath;
}

bool InputFile::isValid()
{
    QFileInfo file(_filePath);
    return file.exists() && file.isReadable();
}

void InputFile::filePath(QString &filePath)
{
    _filePath = QFileInfo(filePath.trimmed()).canonicalFilePath();
    emit filePathChanged(_filePath);
}
