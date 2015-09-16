#ifndef UNITTEST_H
#define UNITTEST_H

#include <QObject>

class UnitTest : public QObject
{
    Q_OBJECT
public:
    explicit UnitTest(QObject *parent = 0);

signals:

public slots:
};

#endif // UNITTEST_H
