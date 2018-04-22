#ifndef _FILEBANK_H_
#define _FILEBANK_H_



#include <QtCore>
#include <QDir>
#include <QFile>
#include <QDirIterator>
#include <QtAlgorithms>


#include "scr.h"
#include "si_globals.h"
#include "fbiterator.h"

// a class handling all the file operations: listing files in source directory,
// managing temporary directory (used to store the results and some marks until the user confirms the changes)
// processing the changes from temporary directory to the destination


class FileBank {
// Q_OBJECT


public:
  FileBank(const QString & source_path, const QStringList & file_mask = { "*.jpg", "*.JPG" });
  ~FileBank() {  }

  FBIterator * iterator() { return new FBIterator(filelist.begin(), filelist.end()); }

  QString getTmpNameFor(const QString & file); // does not check if exists !

  void markAsDeleted(const QString & file);
  void unmarkDeleted(const QString & file);
  bool isMarkedAsDeleted(const QString & file);
  void markRenamed(const QString & file, const QString & prefix);

  void finalizeTmpDir(); // complicated, see source

protected:
  QDir srcDir, tmpDir, dstDir;
  QMap<QString, ScaleCropRule> filelist;




  //static const char * default_file_mask[] = ;

};




#endif // #ifndef _FILEBANK_H_
