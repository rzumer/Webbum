#include "inputfile.h"

InputFile::InputFile(QObject *parent, QString inputFilePath) : QObject(parent)
{
    _filePath = inputFilePath;

    AVFormatContext *formatContext = NULL;
    if(avformat_open_input(&formatContext,inputFilePath.toStdString().c_str(),NULL,NULL) == 0)
    {
        if(avformat_find_stream_info(formatContext,NULL) >= 0)
        {
            // load streams and their information
            for(int i = 0; (unsigned)i < formatContext->nb_streams; i++)
            {
                AVStream *currentStream = formatContext->streams[i];
                const InputStream *localStream = InputStream(this, *currentStream);
                _streams.insert(i, localStream);
            }
        }
    }

    avformat_close_input(&formatContext);
}

bool InputFile::isValid()
{
    QFileInfo file(_filePath);
    return file.exists() && file.isReadable();
}

void InputFile::dumpStreamInformation()
{
    AVFormatContext *formatContext = NULL;
    if(avformat_open_input(&formatContext,_filePath.toStdString().c_str(),NULL,NULL) == 0)
    {
        if(avformat_find_stream_info(formatContext,NULL) >= 0)
        {
            const char *fileNameStdString = _filePath.toStdString().c_str();
            av_dump_format(formatContext,0,fileNameStdString,false);
        }
    }

    avformat_close_input(&formatContext);
}
