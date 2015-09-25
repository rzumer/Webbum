#include "inputfile.h"

InputFile::InputFile(QObject *parent, QString inputFilePath) : QObject(parent)
{
    av_register_all();

    _filePath = QFileInfo(inputFilePath.trimmed()).canonicalFilePath();

    AVFormatContext *formatContext = NULL;

    if(avformat_open_input(&formatContext,inputFilePath.toStdString().c_str(),NULL,NULL) == 0)
    {
        if(avformat_find_stream_info(formatContext,NULL) >= 0)
        {
            // Streams
            for(int i = 0; (unsigned)i < formatContext->nb_streams; i++)
            {
                AVStream *currentStream = formatContext->streams[i];
                InputStream localStream = InputStream(currentStream);
                _streams.insert(i, localStream);
            }
        }

        // Chapters
        for(int i = 0; (unsigned)i < formatContext->nb_chapters; i++)
        {
            AVChapter *currentChapter = formatContext->chapters[i];
            InputChapter localChapter = InputChapter(currentChapter);
            _chapters.insert(i, localChapter);
        }

        // Duration
        _duration = QTime(0,0).addMSecs((double)(formatContext->duration + 500) / AV_TIME_BASE * 1000);

        // Bit Rate
        _bitRate = formatContext->bit_rate;
    }
    avformat_close_input(&formatContext);
}

bool InputFile::isValid()
{
    QFileInfo file(_filePath);
    return !_streams.isEmpty() && file.exists() && file.isReadable();
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
