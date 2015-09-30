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
    enum StreamType {VIDEO, AUDIO, SUBTITLE};
    explicit InputStream(AVStream *stream = new AVStream());

    // getters
    int id() const { return _id; }
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
    QString channelLayout() const { return _channelLayout; }

    bool isValid() { return _id != -1; }

private:
    int _id;
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
    QString _channelLayout; // audio only
};

#endif // INPUTSTREAM_H
