#include "inputchapter.h"

InputChapter::InputChapter(AVChapter *chapter, int id)
{
    // ID
    _id = id;

    // Title
    AVDictionaryEntry *title = av_dict_get(chapter->metadata, "title", nullptr, 0);
    if(title)
        _title = QString::fromStdString(title->value);

    // Start Time/End Time
    _startTime = QTime(0,0).addMSecs(static_cast<int>(round((static_cast<double>(chapter->start) + 500) * av_q2d(chapter->time_base) * 1000)));
    _endTime = QTime(0,0).addMSecs(static_cast<int>(round((static_cast<double>(chapter->end) + 500) * av_q2d(chapter->time_base) * 1000)));
}
