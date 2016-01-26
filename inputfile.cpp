#include "inputfile.h"

InputFile::InputFile(QObject *parent, QString inputFilePath) : QObject(parent)
{
    _width = 0;
    _height = 0;

    _filePath = QFileInfo(inputFilePath.trimmed()).canonicalFilePath();
    emit inputFileChanged(_filePath);

    if(isValid())
    {
        av_register_all();

        AVFormatContext *formatContext = NULL;

        if(avformat_open_input(&formatContext,inputFilePath.toStdString().c_str(),NULL,NULL) == 0)
        {
            if(avformat_find_stream_info(formatContext,NULL) >= 0)
            {
                int videoStreamIndex = 0;
                int audioStreamIndex = 0;
                int subtitleStreamIndex = 0;

                // Streams
                for(int i = 0; (unsigned)i < formatContext->nb_streams; i++)
                {
                    AVStream *currentStream = formatContext->streams[i];
                    InputStream localStream;

                    if(currentStream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
                        localStream = InputStream(currentStream, videoStreamIndex++);
                    else if(currentStream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
                        localStream = InputStream(currentStream, audioStreamIndex++);
                    else if(currentStream->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
                        localStream = InputStream(currentStream, subtitleStreamIndex++);
                    else
                        localStream = InputStream(currentStream);

                    if(localStream.width() + localStream.height() > _width + _height)
                    {
                        _width = localStream.width();
                        _height = localStream.height();
                    }

                    _streams.insert(i, localStream);
                }
            }

            // Chapters
            for(int i = 0; (unsigned)i < formatContext->nb_chapters; i++)
            {
                AVChapter *currentChapter = formatContext->chapters[i];
                InputChapter localChapter = InputChapter(currentChapter, i);
                _chapters.insert(i, localChapter);
            }

            // Duration
            _duration = QTime(0,0).addMSecs((double)(formatContext->duration + 500) / AV_TIME_BASE * 1000);

            // Bit Rate
            _bitRate = formatContext->bit_rate;
        }
        avformat_close_input(&formatContext);
    }
}

double InputFile::fileSize(double durationInMSecs) const
{
    if(durationInMSecs > 0)
        return ((double)_bitRate / 8) * durationInMSecs / 1000;
    else
        return ((double)_bitRate / 8) * QTime(0,0).msecsTo(_duration) / 1000;
}

double InputFile::fileSize(QTime duration) const
{
    return fileSize(QTime(0,0).msecsTo(duration));
}

double InputFile::fileSizeInMegabytes(double durationInMSecs) const
{
    return fileSize(durationInMSecs) / 1024 / 1024;
}

double InputFile::fileSizeInMegabytes(QTime duration) const
{
    return fileSizeInMegabytes(QTime(0,0).msecsTo(duration));
}

bool InputFile::isValid(QString inputFilePath)
{
    QFileInfo file(inputFilePath.trimmed());
    return file.exists() && file.isReadable();
}

bool InputFile::isValid()
{
    return isValid(_filePath);
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
