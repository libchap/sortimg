#include <QtCore>
#include <QHash>
#include <QString>
#include <QTime>





void allocated(const void * ptr, long size, const QString & comment);
void freed(const void * ptr);
void printAllocs();

