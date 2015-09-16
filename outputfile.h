#ifndef OUTPUTFILE_H
#define OUTPUTFILE_H

#include <QObject>

class OutputFile : public QObject
{
    Q_OBJECT
public:
    explicit OutputFile(QObject *parent = 0);

signals:

public slots:
};

#endif // OUTPUTFILE_H
