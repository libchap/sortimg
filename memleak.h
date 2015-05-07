#include <QtCore>
#include <QHash>
#include <QString>
#include <QTime>





void allocated(const void * ptr, const QString & comment);
void freed(const void * ptr);
void printAllocs();

