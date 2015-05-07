#include "memleak.h"


static QHash<const void *, QString> allocs;


void allocated(const void * ptr, const QString & comment) {
  allocs.insert(ptr, comment + "@" + QTime::currentTime().toString());
}
  

  
void freed(const void * ptr) {
  if (allocs.remove(ptr) != 1) {
    qDebug()<<"!!!!EPIC: freeing non-existing ptr";
  }
}

void printAllocs() {
  QHashIterator<const void *, QString> it(allocs);
  
  while (it.hasNext()) {
    it.next();
    qDebug()<<"#"<<it.key()<<" "<<it.value();
  }
}


