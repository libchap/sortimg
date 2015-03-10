#ifndef _IMAGEBUFFER_H_
#define _IMAGEBUFFER_H_



#include <QtCore>
#include <QHash>
#include <QFuture>
#include <QImage>

class ImageBuffer;

#include "scr.h"
#include "si_globals.h"
#include "filebank.h"

#include "qtopia_exif/qexifimageheader.h"




class ImageBuffer {
// Q_OBJECT


protected:

  class IBData {
  public:
    IBData(const QString & filename);
    ~IBData();

    void prepareSC(ScaleCropRule scr, bool fast = false);
    QImage * getSC(ScaleCropRule scr);
    void unprepareSC(ScaleCropRule scr);

    void fileSC(ScaleCropRule scr, const QString & targetFile);

    QSize getOriginalSize();

    void waitForFileRescaling();


  protected:
    QFuture<QImage *> originalImage;
    QExifImageHeader exifData;
    // int rotate; // modulo 4; 0 = keep, 1 = left, 2 = flip, 3 = right; default from exif, changable manually
    QHash<QString, QFuture<QImage *> > rescales; // indexed by ScaleCropRule::toString()

    QFuture<bool> fileRescaling;
    bool fileRescalingRunning = false;


  };


public:
  ~ImageBuffer();
  void addImage(const QString & fileName);
  void addRange(FBIterator && range);
  void removeImage(const QString & fileName);
  void prepareRescale(const QString & fileName, ScaleCropRule scr);
  QImage * getRescaled(const QString & fileName, ScaleCropRule scr);
  void prepareRescale(const QString & fileName) { prepareRescale(fileName, default_scr); }
  QImage * getRescaled(const QString & fileName) { return getRescaled(fileName, default_scr); }
  QSize getOriginalSize(const QString & fileName);
  void rescaleToFile(const QString & fileName, ScaleCropRule scr, const QString & targetFile);
  void waitForFileRescales();

  ScaleCropRule default_scr; // no need to have this private

protected:
  QHash<QString, IBData *> images;



};




#endif // #ifndef _IMAGEBUFFER_H_
