#ifndef INPUTCHAPTER_H
#define INPUTCHAPTER_H

#include <QObject>

class InputChapter : public QObject
{
    Q_OBJECT
public:
    explicit InputChapter(QObject *parent = 0, QString title = QString(), QTime startTime = QTime(0,0), QTime endTime = QTime(0,0));

    // getters
    QString title() const { return _title; }
    QTime startTime() const { return _startTime; }
    QTime endTime() const { return _endTime; }

private:
    QString _title;
    QTime _startTime;
    QTime _endTime;

signals:

public slots:
    void setTitle(QString &title) { _title = title; }
    void setStartTime(QTime &startTime) { _startTime = startTime; }
    void setEndTime(QTime &endTime) { _endTime = endTime; }
};

#endif // INPUTCHAPTER_H
