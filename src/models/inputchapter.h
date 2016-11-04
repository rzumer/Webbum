#ifndef INPUTCHAPTER_H
#define INPUTCHAPTER_H

#include <QObject>
#include <QTime>
extern "C"
{
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
}

class InputChapter
{
public:
    explicit InputChapter(AVChapter *chapter = new AVChapter(), int id = -1);

    // getters
    int id() const { return _id; }
    QString title() const { return _title; }
    QTime startTime() const { return _startTime; }
    QTime endTime() const { return _endTime; }

private:
    int _id;
    QString _title;
    QTime _startTime;
    QTime _endTime;
};

#endif // INPUTCHAPTER_H
