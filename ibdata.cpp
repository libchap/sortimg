#include "imagebuffer.h"

#include <algorithm> // max, min
using std::min;
using std::max;

static QImage * rotate90(const QImage * src) {
    QImage * dst = new QImage(src->height(), src->width(), src->format());
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
    for (int y=0;y<src->height();++y) {
        const uint *srcLine = reinterpret_cast< const uint * >(src->scanLine(y));
        for (int x=0;x<src->width();++x) {
            dst->setPixel(y, src->width()-x-1, srcLine[x]);
        }
    }
    return dst;
}

static void saveExifModified(const QString & targetJpeg, QExifImageHeader * exif, const QSize & jpegSize) {
  QExifValue backup_orientation = exif->value(QExifImageHeader::Orientation),
             backup_pixelXdim = exif->value(QExifImageHeader::PixelXDimension),
             backup_pixelYdim = exif->value(QExifImageHeader::PixelYDimension);

  exif->setValue(QExifImageHeader::Orientation, QExifValue(quint32(1)));
  exif->setValue(QExifImageHeader::PixelXDimension, QExifValue(quint32(jpegSize.width())));
  exif->setValue(QExifImageHeader::PixelYDimension, QExifValue(quint32(jpegSize.height())));

  exif->saveToJpeg(targetJpeg);

  exif->setValue(QExifImageHeader::Orientation, backup_orientation);
  exif->setValue(QExifImageHeader::PixelXDimension, backup_pixelXdim);
  exif->setValue(QExifImageHeader::PixelYDimension, backup_pixelYdim);
}

static QImage * loadImageFromJpeg(QString jpegName, int rotate) {
  QImage * qi = new QImage(jpegName);
  QImage * qir;
  switch ((rotate + 4) % 4) {
    case 1:
      qir = rotate90(qi);
      delete qi;
      break;
    case 2:
      qir = rotate270(qi);
      delete qi;
      break;
    default:
      qir = qi;
  }
  return qir;
}


static QImage * scaleCropImage(QFuture<QImage *> & futureOriginal, ScaleCropRule scr, bool fast) {
  QImage * rotated = futureOriginal.result();
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
        delete rot1;
      }
      break;
  }
  QImage scaled = rotated->scaled(
    scr.scaleSize(),
    Qt::KeepAspectRatio,
    (fast ? Qt::FastTransformation : Qt::SmoothTransformation)
  );
  if (scr.ini_rot != 0) delete rotated;
  QRect basecr = scr.cropRect(), cr;
  QSize scaleds = scaled.size();
  cr.setX(max(basecr.x(), 0));
  cr.setY(max(basecr.y(), 0));
  cr.setWidth(min(basecr.width(), scaleds.width() - cr.x()));
  cr.setHeight(min(basecr.height(), scaleds.height() - cr.y()));
  QImage * cropped = new QImage(scaled.copy(cr));
  // FIXME: copy only the existing subrect, not to fill with black...
  return cropped;
}


static bool scaleCropToFile(QFuture<QImage *> & futureOriginal, ScaleCropRule scr, bool fast, QString target, QExifImageHeader * exif) {
  QImage * sced = scaleCropImage(futureOriginal, scr, fast);
  sced->save(target, 0, si_settings_output_jpeg_quality);
  //QFile::copy(target, target + ".woExif.jpg"); // debug
  saveExifModified(target, exif, sced->size());
  delete sced;
  return true; // just stuff
}




ImageBuffer::IBData::IBData(const QString & filename) : exifData(filename) {
  int rotate = 0;
  if (exifData.contains(QExifImageHeader::Orientation)) {
    switch (exifData.value(QExifImageHeader::Orientation).toShort()) {
      case 6: rotate = 1; break;
      case 8: rotate = 3; break;
    }
  }

  originalImage = QtConcurrent::run(loadImageFromJpeg, filename, rotate);
}


ImageBuffer::IBData::~IBData() {
  QHashIterator<QString, QFuture<QImage *> > i(rescales);
  while (i.hasNext()) {
    QString k = i.next().key();
    delete rescales[k].result();
  }
  waitForFileRescaling();
}


void ImageBuffer::IBData::prepareSC(ScaleCropRule scr, bool fast) {
  QString scrs = scr.toString();
  qDebug() << "prepare: " << scrs;
  if (rescales.contains(scrs)) return;
  rescales.insert(
    scrs,
    QtConcurrent::run(scaleCropImage, originalImage, scr, fast)
  );
}

QImage * ImageBuffer::IBData::getSC(ScaleCropRule scr) {
  QString scrs = scr.toString();
  qDebug() << "get: " << scrs;
  if (!rescales.contains(scrs)) {
    prepareSC(scr); // keep slow. it actually shouldn't happen. we could provide fast but the question is
                    // 1) the pointer 2) keeping crap buffered
  }
  return rescales[scrs].result();
}

void ImageBuffer::IBData::unprepareSC(ScaleCropRule scr) {
  QString scrs = scr.toString();
  if (!rescales.contains(scrs)) return;
  delete rescales[scrs].result();
  rescales.remove(scrs);
}



void ImageBuffer::IBData::fileSC(ScaleCropRule scr, const QString & targetFile) {
  waitForFileRescaling();
  fileRescalingRunning = true;
  fileRescaling = QtConcurrent::run(scaleCropToFile, originalImage, scr, false, targetFile, &exifData);
}


void ImageBuffer::IBData::waitForFileRescaling() {
  if (fileRescalingRunning) {
    bool stuff = fileRescaling.result();
    stuff = stuff; // we do nothing
    fileRescalingRunning = false;
  }
}

QSize ImageBuffer::IBData::getOriginalSize() {
  int w, h;
  if (exifData.contains(QExifImageHeader::PixelXDimension) && exifData.contains(QExifImageHeader::PixelYDimension)) {
    w = (int) exifData.value(QExifImageHeader::PixelXDimension).toLong();
    h = (int) exifData.value(QExifImageHeader::PixelYDimension).toLong();
  }
  else {
    w = originalImage.result()->width();
    h = originalImage.result()->height();
  }
  return QSize(w, h);
}
