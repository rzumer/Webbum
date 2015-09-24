#include "inputstream.h"

InputStream::InputStream(AVStream *stream)
{
    _isDefault = false;

    if(stream)
    {
        AVDictionaryEntry *language = av_dict_get(stream->metadata,"language",NULL,0);
        if(language)
            _language = language->value;

        if(stream->disposition & AV_DISPOSITION_DEFAULT)
            _isDefault = true;

        if(stream->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            _type = VIDEO;
            ui->streamVideoComboBox->addItem(streamStr);
        }
        else if(stream->codec->codec_type==AVMEDIA_TYPE_AUDIO)
        {
            _type = AUDIO;
            ui->streamAudioComboBox->addItem(streamStr);
        }
        else if(stream->codec->codec_type==AVMEDIA_TYPE_SUBTITLE)
        {
            _type = SUBTITLE;
            ui->streamSubtitlesComboBox->addItem(streamStr);
        }

        if(stream->codec->codec_descriptor != NULL)
        {
            QString streamStr = "[" + QString::number(stream->id) + "] ";

            // add stream title if available
            AVDictionaryEntry *title = av_dict_get(stream->metadata,"title",NULL,0);
            if(title)
                streamStr.append("\"" + QString::fromStdString(title->value) + "\" - ");

            // add codec name
            streamStr.append(QString::fromStdString(avcodec_get_name(currentStream->codec->codec_id)));

            // add profile name if available
            if(currentStream->codec->profile != FF_PROFILE_UNKNOWN)
            {
                const AVCodec *profile;
                const char *profileName;

                if(currentStream->codec->codec)
                    profile = currentStream->codec->codec;
                else
                    profile = avcodec_find_decoder(currentStream->codec->codec_id);

                if(profile)
                    profileName = av_get_profile_name(profile,currentStream->codec->profile);

                if(profileName)
                    streamStr.append("/" + QString::fromStdString(profileName) + "");
            }

            // add bitrate and channels to audio streams
            if(currentStream->codec->codec_type==AVMEDIA_TYPE_AUDIO)
            {
                streamStr.append(" (");

                int bitsPerSample = av_get_bits_per_sample(currentStream->codec->codec_id);
                int bitRate = bitsPerSample ? currentStream->codec->sample_rate *
                                              currentStream->codec->channels *
                                              bitsPerSample : currentStream->codec->bit_rate;
                if(bitRate != 0)
                    streamStr.append(QString::number((double)bitRate / 1000) + "kbps/");

                char buf[256];
                av_get_channel_layout_string(buf,sizeof(buf),
                    currentStream->codec->channels,currentStream->codec->channel_layout);
                streamStr.append(QString::fromStdString(buf) + ")");
            }
        }
    }
}

