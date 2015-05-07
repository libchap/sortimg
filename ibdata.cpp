#include "imagebuffer.h"

#include <algorithm> // max, min
using std::min;
using std::max;

// counter-clockwise
static QImage * rotate90(const QImage * src) {
    QImage * dst = new QImage(src->height(), src->width(), src->format());
    allocated(dst, "rotate90");
    for (int y=0;y<src->height();++y) {
        const uint *srcLine = reinterpret_cast< const uint * >(src->scanLine(y));
        for (int x=0;x<src->width();++x) {
            dst->setPixel(src->height()-y-1, x, srcLine[x]);
        }
    }
    return dst;
}
static QImage * rotate270(const QImage * src) {
    QImage * dst = new QImage(src->height(), src->width(), src->format());
    allocated(dst, "rotate270");
    for (int y=0;y<src->height();++y) {
        const uint *srcLine = reinterpret_cast< const uint * >(src->scanLine(y));
        for (int x=0;x<src->width();++x) {
            dst->setPixel(y, src->width()-x-1, srcLine[x]);
        }
    }
    return dst;
}

// fix orientation and resolution tags in exif according to current situation
// save the exif to the jpeg file created by QImage::save
static void saveExifModified(const QString & targetJpeg, const StupidExif & exif, const QSize & jpegSize) {
  StupidExif temp_exif(exif);

  temp_exif.setTagOrientation(1);
  temp_exif.setTagXResolution(jpegSize.width());
  temp_exif.setTagYResolution(jpegSize.height());

  temp_exif.insert_into_jpeg(targetJpeg);
}

static QImage * loadImageFromJpeg(QString jpegName, int rotate) {
  QImage * qi = new QImage(jpegName);
  allocated(qi, "loadImageFromJpeg");
  QImage * qir;
  switch ((rotate + 4) % 4) {
    case 1:
      qir = rotate90(qi);
      freed(qi);
      delete qi;
      break;
    case 2:
      qir = rotate270(qi);
      freed(qi);
      delete qi;
      break;  // FIXME: missing case 3 ?
    default:
      qir = qi;
  }
  return qir;
}

// grab original (unresized) image data from a QFuture, apply SCR on it (rotate, resize, crop)
static QImage * scaleCropImage(QFuture<QImage *> & futureOriginal, ScaleCropRule scr, bool fast) {
  QImage * rotated = futureOriginal.result();
  bool delete_rotated = true;
  qDebug() << "Rotating: " << scr.ini_rot;
  switch (scr.ini_rot) {
    case 1:
      rotated = rotate270(rotated);
      break;
    case 3:
      rotated = rotate90(rotated);
      break;
    case 2: {
        QImage * rot1 = rotate90(rotated);
        rotated = rotate90(rot1);
	freed(rot1);
        delete rot1;
      }
      break;
    default:
      delete_rotated = false;
      break;
  }
  QImage scaled = rotated->scaled(
    scr.scaleSize(),
    Qt::KeepAspectRatio,
    (fast ? Qt::FastTransformation : Qt::SmoothTransformation)
  );
  if (scr.ini_rot != 0) {
    freed(rotated);
    delete rotated;
  }
  QRect basecr = scr.cropRect(), cr;
  QSize scaleds = scaled.size();
  cr.setX(max(basecr.x(), 0));
  cr.setY(max(basecr.y(), 0));
  cr.setWidth(min(basecr.width(), scaleds.width() - cr.x()));
  cr.setHeight(min(basecr.height(), scaleds.height() - cr.y()));
  QImage * cropped = new QImage(scaled.copy(cr));
  allocated(cropped, "cropped");
  // FIXME: copy only the existing subrect, not to fill with black...
  if (delete_rotated) {
    freed(rotated);
    delete rotated;
  }
  return cropped;
}

// the same as before but save the result into file (together with provided exif)
static bool scaleCropToFile(QFuture<QImage *> & futureOriginal, ScaleCropRule scr, bool fast, QString target, const StupidExif & exif) {
  QImage * sced = scaleCropImage(futureOriginal, scr, fast);
  sced->save(target, 0, si_settings_output_jpeg_quality);
  //QFile::copy(target, target + ".woExif.jpg"); // debug
  saveExifModified(target, exif, sced->size());
  freed(sced);
  delete sced;
  return true; // just stuff
}



// load image and exif from a file, load the unresized original image data into QFuture
ImageBuffer::IBData::IBData(const QString & filename) : exifData(filename) {
  int rotate = 0;
  switch (exifData.getTagOrientation()) {
    case 6: rotate = 1; break;
    case 8: rotate = 3; break;
  }

  originalImage = QtConcurrent::run(loadImageFromJpeg, filename, rotate);
}


ImageBuffer::IBData::~IBData() {
  freed(originalImage.result());
  delete originalImage.result();
  
  QHashIterator<QString, QFuture<QImage *> > i(rescales);
  while (i.hasNext()) {
    QString k = i.next().key();
    freed(rescales[k].result());
    delete rescales[k].result();
  }
  waitForFileRescaling();
}

// for the loaded image, run the rescale in the baground to be (possibly) used later
void ImageBuffer::IBData::prepareSC(ScaleCropRule scr, bool fast) {
  QString scrs = scr.toString();
  qDebug() << "prepare: " << scrs;
  if (rescales.contains(scrs)) return;
  rescales.insert(
    scrs,
    QtConcurrent::run(scaleCropImage, originalImage, scr, fast)
  );
}

// grab the prepared resized image (resize immediately if not prepared)
QImage * ImageBuffer::IBData::getSC(ScaleCropRule scr) {
  QString scrs = scr.toString();
  qDebug() << "get: " << scrs;
  if (!rescales.contains(scrs)) {
    prepareSC(scr); // keep slow. it actually shouldn't happen. we could provide fast but the question is
                    // 1) the pointer 2) keeping crap buffered
  }
  return rescales[scrs].result();
}

// cleanup. for the sake of not having to many different resizes in the memory
void ImageBuffer::IBData::unprepareSC(ScaleCropRule scr) {
  QString scrs = scr.toString();
  if (!rescales.contains(scrs)) return;
  freed(rescales[scrs].result());
  delete rescales[scrs].result();
  rescales.remove(scrs);
}


// "prepareSC" into a file
void ImageBuffer::IBData::fileSC(ScaleCropRule scr, const QString & targetFile) {
  waitForFileRescaling();
  fileRescalingRunning = true;
  fileRescaling = QtConcurrent::run(scaleCropToFile, originalImage, scr, false, targetFile, exifData);
}

// check if all the fileSC have finished, wait if not
void ImageBuffer::IBData::waitForFileRescaling() {
  if (fileRescalingRunning) {
    bool stuff = fileRescaling.result();
    stuff = stuff; // we do nothing
    fileRescalingRunning = false;
  }
}

QSize ImageBuffer::IBData::getOriginalSize() {
  int w, h;
  w = exifData.getTagXResolution();
  h = exifData.getTagYResolution();
  if (w == 0 || h == 0) {
    w = originalImage.result()->width();
    h = originalImage.result()->height();
  }
  return QSize(w, h);
}
