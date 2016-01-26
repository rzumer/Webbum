#ifndef INPUTSTREAM_H
#define INPUTSTREAM_H

#include <QObject>
extern "C"
{
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
}

class InputStream
{
public:
    enum StreamType {VIDEO, AUDIO, SUBTITLE, OTHER};
    explicit InputStream(AVStream *stream = NULL, int index = -1);

    // getters
    int id() const { return _id; }
    int index() const { return _index; }
    QString codec() const { return _codec; }
    QString profile() const { return _profile; }
    QString title() const { return _title; }
    QString language() const { return _language; }
    bool isDefault() const { return _isDefault; }
    bool isForced() const { return _isForced; }
    int type() const { return _type; }
    double frameRate() const { return _frameRate; }
    int width() const { return _width; }
    int height() const { return _height; }
    int bitRate() const { return _bitRate; }
    int channels() const { return _channels; }
    QString channelLayout() const { return _channelLayout; }
    bool isValid() { return _index != -1; }
    bool isImageSub() const;

private:
    int _id;
    int _index;
    QString _codec;
    QString _profile;
    QString _title;
    QString _language;
    bool _isDefault;
    bool _isForced;
    StreamType _type; // video/audio/subtitle only
    double _frameRate; // video only
    int _width; // video only
    int _height; // video only
    int _bitRate; // audio only
    int _channels; // audio only
    QString _channelLayout; // audio only
};

#endif // INPUTSTREAM_H
