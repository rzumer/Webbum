#ifndef INPUTCHAPTER_H
#define INPUTCHAPTER_H

#include <QObject>
#include <QTime>

class InputChapter
{
public:
    explicit InputChapter(QString title = QString(), QTime startTime = QTime(0,0), QTime endTime = QTime(0,0));

    // getters
    QString title() const { return _title; }
    QTime startTime() const { return _startTime; }
    QTime endTime() const { return _endTime; }

    // setters
    void setTitle(QString &title) { _title = title; }
    void setStartTime(QTime &startTime) { _startTime = startTime; }
    void setEndTime(QTime &endTime) { _endTime = endTime; }

private:
    QString _title;
    QTime _startTime;
    QTime _endTime;
};

#endif // INPUTCHAPTER_H
