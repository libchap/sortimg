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

#include "stupid_exif.h"
#include "memleak.h"

#include "colorop.h"

// class for managing and storing image data
// for a subset of images in FileBank (only the ones user is gonna view in near future (past))
// storing original images as well as various resized variants for each of them
// also storing and managing exif


class ImageBuffer {
// Q_OBJECT

public:
  typedef enum {
    QTFAST, QTSLOW, RLANCZOS4
  } RecaleMethod;

protected:
  class IBData {
    // for a single input file, store its various resizes,
    // do the resizes as asked by the user
    // do all the expensive operation in separate threads (using QFuture)
    // also store and handle exif data for the single image
  public:
    IBData(const QString & filename);
    ~IBData();

    void prepareSC(ScaleCropRule scr, RecaleMethod method = RLANCZOS4);
    QImage * getSC(ScaleCropRule scr);
    void unprepareSC(ScaleCropRule scr);

    void fileSC(ScaleCropRule scr, const QString & targetFile);

    QSize getOriginalSize();

    void waitForFileRescaling();
    void waitForAllRescaling();

  protected:
    QFuture<QImage *> originalImage;
    StupidExif exifData;
    // int rotate; // modulo 4; 0 = keep, 1 = left, 2 = flip, 3 = right; default from exif, changable manually
    QHash<QString, QFuture<QImage *> > rescales; // indexed by ScaleCropRule::toString()

    QFuture<bool> fileRescaling;
    volatile bool fileRescalingRunning = false;


  };


public:
  ~ImageBuffer();
  void addImage(const QString & fileName, bool prepare = true);
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
