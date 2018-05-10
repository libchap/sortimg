#include "memleak.h"


static QHash<const void *, QPair<long, QString> > allocs;


void allocated(const void * ptr, long size, const QString & comment) {
  allocs.insert(ptr, QPair<long, QString>(size, comment + "@" + QTime::currentTime().toString()));
}
  

  
void freed(const void * ptr) {
  if (allocs.remove(ptr) != 1) {
    qDebug()<<"!!!!EPIC: freeing non-existing ptr";
  }
}

void printAllocs() {
  QHashIterator<const void *, QPair<long, QString> > it(allocs);
  
  while (it.hasNext()) {
    it.next();
    qDebug()<<"#"<<it.key()<<" "<<it.value().first<<" "<<it.value().second;
  }
}


