#ifndef _STUPID_EXIF_H_
#define _STUPID_EXIF_H_

#include <QString>
#include <QFile>
#include <QDataStream>
#include <QVector>
#include <QDebug>

#include "memleak.h"



// this is something like a library
// it handles whole exif part of jpeg image
// read it, insert it into another existing jpeg file
// and modify the resolution and orientation tags (if detected)
// it is not more clever than necessary
// all the errors are silent-failed ! no chance to detect errors !



typedef char BYTE, * PBYTE;


typedef enum {
  EINTEL = 0x4949,
  EMOTOROLA = 0x4d4d
} ENDIAN, * PENDIAN;


class StupidExif {

public:
  StupidExif(const QString & jpg) { read_from_jpeg(jpg); }
  virtual ~StupidExif() { if (exifData) { freed(exifData); delete [] exifData; } }
  StupidExif(const StupidExif & se) { se_copy_from(se); }
  StupidExif & operator=(const StupidExif & se) { if (exifData) { freed(exifData); delete [] exifData; } return se_copy_from(se); }

  void insert_into_jpeg(const QString & jpegName) const;

  int getTagOrientation() const;
  void setTagOrientation(int x);
  int getTagXResolution() const;
  void setTagXResolution(int x);
  int getTagYResolution() const;
  void setTagYResolution(int x);

  int getExifDataLen() const;
private:
  void read_from_jpeg(const QString & jpegName);
  void decodeIFD(PBYTE ifd, long offset_delta);

  ENDIAN exifEndian;
  PBYTE exifData = NULL;
  PBYTE tagOrientation1 = NULL;
  PBYTE tagOrientation2 = NULL;
  PBYTE tagXResolution = NULL;
  PBYTE tagYResolution = NULL;

  StupidExif & se_copy_from(const StupidExif & se);

};








#endif // #define _STUPID_EXIF_H_
