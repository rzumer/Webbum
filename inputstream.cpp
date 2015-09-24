#include "inputstream.h"

InputStream::InputStream(AVStream *stream)
{
    _isDefault = false;
    _isForced = false;

    // Extract stream information
    if(stream)
    {
        // ID
        _id = stream->id;

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
        if(stream->codec->codec_type==AVMEDIA_TYPE_VIDEO)
            _type = VIDEO;
        else if(stream->codec->codec_type==AVMEDIA_TYPE_AUDIO)
            _type = AUDIO;
        else if(stream->codec->codec_type==AVMEDIA_TYPE_SUBTITLE)
            _type = SUBTITLE;

        // Additional information
        if(stream->codec->codec_descriptor)
        {
            // Title
            AVDictionaryEntry *title = av_dict_get(stream->metadata,"title",NULL,0);
            if(title)
                _title = title;

            // Codec Name
            _codec = QString::fromStdString(avcodec_get_name(currentStream->codec->codec_id));

            // Profile Name
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
                    _profile = profileName;
            }

            // Video information
            if(stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                // Frame Rate
                _frameRate = av_q2d(stream->codec->framerate);
            }

            // Audio information
            if(stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                // Bit Rate
                int bitsPerSample = av_get_bits_per_sample(currentStream->codec->codec_id);
                int bitRate = bitsPerSample ? currentStream->codec->sample_rate *
                                              currentStream->codec->channels *
                                              bitsPerSample : currentStream->codec->bit_rate;
                if(bitRate != 0)
                    _bitRate = bitRate;

                // Channel Layout
                char buf[256];
                av_get_channel_layout_string(buf,sizeof(buf),
                    currentStream->codec->channels,currentStream->codec->channel_layout);
                _channelLayout = QString::fromStdString(buf);
            }
        }
    }
}

