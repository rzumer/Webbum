#include "inputstream.h"

InputStream::InputStream(AVStream *stream, int index)
{
    _index = index;
    _isDefault = false;
    _isForced = false;
    _width = 0;
    _height = 0;
    _bitRate = 0;
    _channels = 0;

    // Extract stream information
    if(stream)
    {
        // ID
        _id = stream->id > 0 ? stream->id : stream->index;

        // Language
        AVDictionaryEntry *language = av_dict_get(stream->metadata,"language", nullptr, 0);
        if(language)
            _language = language->value;

        // Disposition
        if(stream->disposition & AV_DISPOSITION_DEFAULT)
            _isDefault = true;
        if(stream->disposition & AV_DISPOSITION_FORCED)
            _isForced = true;

        // Codec Type
        if(stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            _type = VIDEO;
        else if(stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            _type = AUDIO;
        else if(stream->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
            _type = SUBTITLE;
        else
            _type = OTHER;

        // Title
        AVDictionaryEntry *title = av_dict_get(stream->metadata,"title", nullptr, 0);
        if(title)
            _title = QString::fromStdString(title->value);

        // Codec Name
        _codec = QString::fromStdString(avcodec_get_name(stream->codecpar->codec_id));

        // Profile Name
        if(stream->codecpar->profile != FF_PROFILE_UNKNOWN)
        {
            const AVCodec *decoder;
            const char *profileName = nullptr;

            decoder = avcodec_find_decoder(stream->codecpar->codec_id);

            if(decoder)
                profileName = av_get_profile_name(decoder, stream->codecpar->profile);

            if(profileName)
                _profile = profileName;
        }

        // Video information
        if(stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            // Frame Rate
            _frameRate = av_q2d(stream->r_frame_rate);

            // Width/Height
            _width = stream->codecpar->width;
            _height = stream->codecpar->height;
        }

        // Audio information
        if(stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            // Bit Rate
            int bitsPerSample = av_get_bits_per_sample(stream->codecpar->codec_id);
            long long bitRate = bitsPerSample ? stream->codecpar->sample_rate *
                                          stream->codecpar->channels *
                                          bitsPerSample : stream->codecpar->bit_rate;

            _bitRate = std::max(bitRate, 0ll);

            // Channel Layout
            _channels = stream->codecpar->channels;
            char buf[256];
            av_get_channel_layout_string(buf,sizeof(buf),
                stream->codecpar->channels,stream->codecpar->channel_layout);
            _channelLayout = QString::fromStdString(buf);
        }
    }
}

bool InputStream::isImageSub() const
{
    return (_type == StreamType::SUBTITLE && (_codec == "dvd_subtitle" || _codec == "hdmv_pgs_subtitle"));
}

QString InputStream::getShortString() const
{
    QString streamString = "[" + QString::number(_id) + "] ";

    // Title
    if(!_title.isEmpty())
        streamString.append("\"" + _title + "\" - ");

    // Codec
    streamString.append(_codec);

    // Profile
    if(!_profile.isEmpty())
        streamString.append("/" + _profile);

    // Audio information
    if(_type == InputStream::AUDIO && (_bitRate > 0 || !_channelLayout.isEmpty()))
    {
        streamString.append(" (");

        // Bit Rate
        if(_bitRate > 0)
        {
            streamString.append(QString::number(round(static_cast<double>(_bitRate) / 1000)) + "kbps");

            // Channel Layout
            if(!_channelLayout.isEmpty())
                streamString.append("/");
        }

        streamString.append(_channelLayout + ")");
    }

    // Language
    if(!_language.isEmpty())
        streamString.append(" (" + _language + ")");

    // Disposition
    /*if(_isDefault)
        streamStr.append(" [default]");
    if(_isForced)
        streamStr.append(" [forced]");*/

    return streamString;
}

