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
        AVDictionaryEntry *language = av_dict_get(stream->metadata,"language",NULL,0);
        if(language)
            _language = language->value;

        // Disposition
        if(stream->disposition & AV_DISPOSITION_DEFAULT)
            _isDefault = true;
        if(stream->disposition & AV_DISPOSITION_FORCED)
            _isForced = true;

        // Codec Type
        if(stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            _type = VIDEO;
        else if(stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            _type = AUDIO;
        else if(stream->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
            _type = SUBTITLE;
        else
            _type = OTHER;

        // Title
        AVDictionaryEntry *title = av_dict_get(stream->metadata,"title",NULL,0);
        if(title)
            _title = QString::fromStdString(title->value);

        // Codec Name
        _codec = QString::fromStdString(avcodec_get_name(stream->codec->codec_id));

        // Profile Name
        if(stream->codec->profile != FF_PROFILE_UNKNOWN)
        {
            const AVCodec *profile;
            const char *profileName;

            if(stream->codec->codec)
                profile = stream->codec->codec;
            else
                profile = avcodec_find_decoder(stream->codec->codec_id);

            if(profile)
                profileName = av_get_profile_name(profile,stream->codec->profile);

            if(profileName)
                _profile = profileName;
        }

        // Video information
        if(stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            // Frame Rate
            _frameRate = av_q2d(stream->codec->framerate);

            // Width/Height
            _width = stream->codec->width;
            _height = stream->codec->height;
        }

        // Audio information
        if(stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            // Bit Rate
            int bitsPerSample = av_get_bits_per_sample(stream->codec->codec_id);
            int bitRate = bitsPerSample ? stream->codec->sample_rate *
                                          stream->codec->channels *
                                          bitsPerSample : stream->codec->bit_rate;
            if(bitRate > 0)
                _bitRate = bitRate;
            else
                _bitRate = 0;

            // Channel Layout
            _channels = stream->codec->channels;
            char buf[256];
            av_get_channel_layout_string(buf,sizeof(buf),
                stream->codec->channels,stream->codec->channel_layout);
            _channelLayout = QString::fromStdString(buf);
        }
    }
}

bool InputStream::isImageSub() const
{
    return (_type == StreamType::SUBTITLE && (_codec == "dvd_subtitle" || _codec == "hdmv_pgs_subtitle"));
}

