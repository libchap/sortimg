#include "filebank.h"


QString createTmpDir(const char * keyword = "sortimg") {
  QDir temp_parent = QDir::temp();
  QString temp_dirname;
  int i = 0;
  do {
    temp_dirname.sprintf("%s%04d", keyword, i++);
  } while (!temp_parent.mkdir(temp_dirname));
  return temp_parent.absoluteFilePath(temp_dirname);
}

// QString fixPath(const QString & a_path) {
//   QString s(a_path);
//   
//   s.replace("/./", "/");
//   if (s.endsWith("/.")) s.chop(2);
//   
//   return s;
// }
// not used, use QDir::cleanPath instead !

FileBank::FileBank(const QString & source_path, const QStringList & file_mask) 
    : srcDir(QDir::cleanPath(source_path)), tmpDir(createTmpDir()),
      dstDir(QDir::cleanPath(srcDir.absoluteFilePath(si_settings_dest_dir))) {
      
  QDirIterator srcIt(srcDir.absolutePath(), file_mask, QDir::Files, QDirIterator::NoIteratorFlags);
  
  while (srcIt.hasNext()) {
    filelist.insert(srcIt.next(), ScaleCropRule());
  }
  
  //qSort(filelist);
  
  qDebug() << "Source dir: " << srcDir.path();
  qDebug() << "Temp dir: " << tmpDir.path();
  qDebug() << "Target dir: " << dstDir.path();
}


QString FileBank::getTmpNameFor(const QString & file) {
  QString pure_file = srcDir.relativeFilePath(file);
  return tmpDir.absoluteFilePath(pure_file);
}
  
  
  
void FileBank::markAsDeleted(const QString & file) {
  qDebug() << getTmpNameFor(file) + ".deleted";
  
  QFile delFile(getTmpNameFor(file) + ".deleted");
  delFile.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream delStream(&delFile);
  delStream << "Following file is gonna be deleted:\n" << file << "\n";
  delFile.close();
}

void FileBank::unmarkDeleted(const QString & file) {
  QFile delFile(getTmpNameFor(file) + ".deleted");
  delFile.remove();
}

bool FileBank::isMarkedAsDeleted(const QString & file) {
  QFile delFile(getTmpNameFor(file) + ".deleted");
  return delFile.exists();
}



void FileBank::finalizeTmpDir() {
  QMapIterator<QString, ScaleCropRule> allFiles(filelist);
  
  bool sameStrDst = (srcDir.canonicalPath() == dstDir.canonicalPath()); // actually not used
  while (allFiles.hasNext()) {
    QString processed = allFiles.next().key();
    QString pFileName = srcDir.relativeFilePath(processed);
    QString pTName = tmpDir.absoluteFilePath(pFileName);
    QString pTDeleted = pTName + ".deleted";
    QString pDName = dstDir.absoluteFilePath(pFileName);
    bool pTExists = QFile(pTName).exists();
    bool pTDExists = QFile(pTDeleted).exists();
    bool pDNExists = QFile(pDName).exists();
    
    if ((pTExists || pTDExists) && pDNExists) dstDir.remove(pFileName);
    if (pTExists && !pTDExists) tmpDir.rename(pFileName, pDName);
    if (pTExists && pTDExists) tmpDir.remove(pFileName);
    if (pTDExists) tmpDir.remove(pTDeleted);
    if (!sameStrDst && !pTDExists && !pTExists && pDNExists) dstDir.remove(pFileName);
    if (!sameStrDst && !pTDExists && !pTExists) QFile::copy(processed, pDName);
  }
  
  tmpDir.rmdir(".");
}
    
    
    




