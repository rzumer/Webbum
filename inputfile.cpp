#include "inputfile.h"

InputFile::InputFile(QObject *parent, QString inputFilePath) : QObject(parent)
{
    filePath(inputFilePath);
}

bool InputFile::isValid()
{
    QFile file(_filePath);
    return file.exists() && file.isReadable();
}

void InputFile::filePath(QString &filePath)
{
    _filePath = QFileInfo(filePath.trimmed()).canonicalFilePath();
}
