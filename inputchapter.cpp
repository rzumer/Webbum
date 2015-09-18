#include "inputchapter.h"

InputChapter::InputChapter(QObject *parent, QString title, QTime startTime, QTime endTime) : QObject(parent)
{
    _title = title;
    _startTime = startTime;
    _endTime = endTime;
}
