#ifndef INPUTSTREAM_H
#define INPUTSTREAM_H

#include <QObject>

class InputStream : public QObject
{
    Q_OBJECT
public:
    explicit InputStream(QObject *parent = 0);

    // getters
    int type() const { return _type; }
    QString codec() const { return _codec; }
    QString profile() const { return _profile; }
    double frameRate() const { return _frameRate; }
    QString title() const { return _title; }
    QString language() const { return _language; }
    bool isDefault() const { return _isDefault; }

private:
    enum _type {VIDEO,AUDIO,SUBTITLE};
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
