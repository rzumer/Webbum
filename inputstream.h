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
    enum streamType {VIDEO, AUDIO, SUBTITLE};
    explicit InputStream(AVStream *stream = new AVStream());

    // getters
    int id() const { return _id; }
    QString codec() const { return _codec; }
    QString profile() const { return _profile; }
    QString title() const { return _title; }
    QString language() const { return _language; }
    bool isDefault() const { return _isDefault; }
    bool isForced() const { return _isForced; }
    bool isVideo() const { return _type == VIDEO; }
    bool isAudio() const { return _type == AUDIO; }
    bool isSubtitle() const { return _type == SUBTITLE; }
    double frameRate() const { return _frameRate; }
    int bitRate() const { return _bitRate; }
    QString channelLayout() const { return _channelLayout; }

private:
    int _id;
    streamType _type;
    QString _codec;
    QString _profile;
    QString _title;
    QString _language;
    bool _isDefault;
    bool _isForced;
    double _frameRate; // video only
    int _bitRate; // audio only
    QString _channelLayout; // audio only
};

#endif // INPUTSTREAM_H
