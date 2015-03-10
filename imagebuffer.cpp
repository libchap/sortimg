#include "imagebuffer.h"



ImageBuffer::~ImageBuffer() {
  QHashIterator<QString, IBData *> it(images);
  while (it.hasNext()) {
	IBData * to_delete = it.next().value();
	delete to_delete;
  }
}

void ImageBuffer::addImage(const QString & fileName) {
  if (!fileName.isEmpty() && !images.contains(fileName)) {
    images.insert(fileName, new IBData(fileName));
  }
  if (default_scr.scale_w != 0) {
    prepareRescale(fileName);
  }
}

void ImageBuffer::addRange(FBIterator && range) {
  while (range.isValid()) {
    addImage(*range);
    range.next_go();
  }
}

void ImageBuffer::removeImage(const QString & fileName) {
  if (!fileName.isEmpty() && images.contains(fileName)) {
    delete images[fileName];
    images.remove(fileName);
  }
}

void ImageBuffer::prepareRescale(const QString & fileName, ScaleCropRule scr) {
  if (scr.scale_w > 0 && !fileName.isEmpty() && images.contains(fileName)) {
    //qDebug() << "Preparing rescale (" << fileName << ") to : " << scr.toString();
    images[fileName]->prepareSC(scr);
  }
}

void ImageBuffer::rescaleToFile(const QString & fileName, ScaleCropRule scr, const QString & targetFile) {
  if (scr.scale_w > 0 && !fileName.isEmpty() && images.contains(fileName)) {
    images[fileName]->fileSC(scr, targetFile);
  }
}

void ImageBuffer::waitForFileRescales() {
  foreach (IBData * ibd, images) {
    ibd->waitForFileRescaling();
  }
}


QImage * ImageBuffer::getRescaled(const QString & fileName, ScaleCropRule scr) {
  //qDebug() << "getRescaled: " << fileName << "; " << scr.toString() << "; " << images.contains(fileName);
  if (scr.scale_w > 0 && !fileName.isEmpty() && images.contains(fileName)) {
    return images[fileName]->getSC(scr);
  }
  else return NULL;
}

QSize ImageBuffer::getOriginalSize(const QString & fileName) {
  if (!fileName.isEmpty() && images.contains(fileName)) {
    return images[fileName]->getOriginalSize();
  }
  return QSize(0, 0);
}


