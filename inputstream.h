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
    int type() const { return _type; }
    QString codec() const { return _codec; }
    QString profile() const { return _profile; }
    double frameRate() const { return _frameRate; }
    QString title() const { return _title; }
    QString language() const { return _language; }
    bool isDefault() const { return _isDefault; }

private:
    streamType _type;
    QString _codec;
    QString _profile;
    double _frameRate;
    QString _title;
    QString _language;
    bool _isDefault;

signals:

public slots:
    // TODO: add fromAVStream(AVStream **stream) ...
};

#endif // INPUTSTREAM_H
