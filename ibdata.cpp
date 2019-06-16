#include "imagebuffer.h"

// #include <algorithm> // max, min
// using std::min;
// using std::max;


// QSize to QString user readable representation
QString size2string(const QSize & s) {
  if (s.isValid()) {
    QString str;
    return str.sprintf("%dx%d", s.width(), s.height());
  }
  else return "InvalidSize";
}
// reverse
bool string2size(const QString & s, QSize * r) {
  if (s == "InvalidSize") {
    *r = QSize();
    return true;
  }
  QStringList ss = s.split('x');
  if (ss.size() != 2) return false;
  r->setWidth(ss.at(0).toInt());
  r->setHeight(ss.at(1).toInt());
  return true;
}


// counter-clockwise
static QImage * rotate90(const QImage * src) {
    QImage * dst = new QImage(src->height(), src->width(), src->format());
    allocated(dst, sizeof(QImage), "rotate90");
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
    allocated(dst, sizeof(QImage), "rotate270");
    for (int y=0;y<src->height();++y) {
        const uint *srcLine = reinterpret_cast< const uint * >(src->scanLine(y));
        for (int x=0;x<src->width();++x) {
            dst->setPixel(y, src->width()-x-1, srcLine[x]);
        }
    }
    return dst;
}


// this is the usage of stolen 'resampler' library to resample images by various algorithms
#include "resampler.h"
// TODO add switch for various algoritmhs
static QImage scale_with_resampler(const QImage * src, QSize newsize, const char * filter) {
	int src_width = src->width(), src_height = src->height(), dst_width = newsize.width(), dst_height = newsize.height();
	int dst_width_aspect = (int) (src_width * dst_height / (double) src_height + 0.5), dst_height_aspect = (int) (src_height * dst_width / (double) src_width + 0.5);
	if (dst_width_aspect < dst_width) dst_width = dst_width_aspect;
	if (dst_height_aspect < dst_height) dst_height = dst_height_aspect;

	const float filter_scale = 1.0f; /* this can be slightly lower to sharpen */
	Resampler rR(src_width, src_height, dst_width, dst_height, Resampler::BOUNDARY_CLAMP, 0.0f, 1.0f, filter, NULL, NULL, filter_scale, filter_scale);
	Resampler rG(src_width, src_height, dst_width, dst_height, Resampler::BOUNDARY_CLAMP, 0.0f, 1.0f, filter, rR.get_clist_x(), rR.get_clist_y(), filter_scale, filter_scale);
	Resampler rB(src_width, src_height, dst_width, dst_height, Resampler::BOUNDARY_CLAMP, 0.0f, 1.0f, filter, rR.get_clist_x(), rR.get_clist_y(), filter_scale, filter_scale);
	Resampler rA(src_width, src_height, dst_width, dst_height, Resampler::BOUNDARY_CLAMP, 0.0f, 1.0f, filter, rR.get_clist_x(), rR.get_clist_y(), filter_scale, filter_scale);

	int i, j;

	float * lineR = new float[src_width], * lineG = new float[src_width], * lineB = new float[src_width], * lineA = new float[src_width];
	for (i = 0; i < src_height; i++) {
		QRgb * line = (QRgb *) src->scanLine(i);
		for (j = 0; j < src_width; j++) {
			lineR[j] = qRed(line[j]) * (1.0f/255.0f); lineG[j] = qGreen(line[j]) * (1.0f/255.0f); // FIXME: consider gamma correction for RGB channels
			lineB[j] = qBlue(line[j]) * (1.0f/255.0f); lineA[j] = qAlpha(line[j]) * (1.0f/255.0f);
		}
		rR.put_line(lineR); rG.put_line(lineG); rB.put_line(lineB); rA.put_line(lineA); 
	}

	QImage dst(dst_width, dst_height, src->format());
	for (i = 0; i < dst_height; i++) {
		const float * oR = rR.get_line(), * oG = rG.get_line(), * oB = rB.get_line(), * oA = rA.get_line();
		for (j = 0; j < dst_width; j++) {
#define f2ch(ch) ((int) ((ch) * 255.0f + 0.5f))
			QRgb x = qRgba(f2ch(oR[j]), f2ch(oG[j]), f2ch(oB[j]), f2ch(oA[j]) /* or 255 ? ;) */);
			dst.setPixel(j, i, x);
		}
	}
	
	delete [] lineR; delete [] lineG; delete [] lineB; delete [] lineA;
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
  allocated(qi, sizeof(QImage), "loadImageFromJpeg");
  QImage * qir;
  //switch ((rotate + 4) % 4) {
  switch (rotate) {
    case 1:
      qir = rotate90(qi);
      freed(qi);
      delete qi;
      break;
    case 3:
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
static QImage * scaleCropImage(QFuture<QImage *> & futureOriginal, ScaleCropRule scr, ImageBuffer::RecaleMethod method) {
  QImage * rotated = futureOriginal.result();

  qDebug() << "Rotating: " << scr.rotate;
  switch (scr.rotate) {
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

  }
  
  QImage scaled;
  switch (method) {
    case ImageBuffer::QTSLOW:
      scaled = rotated->scaled(scr.scaleSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
      break;
    case ImageBuffer::QTFAST:
      scaled = rotated->scaled(scr.scaleSize(), Qt::KeepAspectRatio, Qt::FastTransformation);
      break;
    case ImageBuffer::RLANCZOS4:
      scaled = scale_with_resampler(rotated, scr.scaleSize(), "lanczos4");
      //qDebug()<<"WARNING: slow Lanczos !!";
      break;
    default:
      scaled = *rotated;
      break;
  }
  
  if (scr.rotate != 0) {
    freed(rotated);
    delete rotated;
  }
  QRect basecr = scr.cropRect();
  QImage cropped1 = scaled.copy(basecr);
  QImage colored = changeBrightness(cropped1, scr.brightness);
  QImage * the_result = new QImage(colored);
  allocated(the_result, sizeof(QImage), "cropped");
  //qDebug()<<"scaled: "<<size2string(scaled.size())<<" ; rect: "<<cr.x()<<":"<<cr.y()<<"/"<<size2string(cr.size())<<" ; cropped1: "<<size2string(cropped1.size());
  qDebug()<<"scaled: "<<size2string(scaled.size())<<" ; rect: "<<basecr.x()<<":"<<basecr.y()<<"/"<<size2string(basecr.size())<<" ; cropped1: "<<size2string(cropped1.size());
  //qDebug()<<"cropped: "<<size2string(cropped->size());
  // FIXME: copy only the existing subrect, not to fill with black...

  return the_result;
}

// the same as before but save the result into file (together with provided exif)
static bool scaleCropToFile(QFuture<QImage *> & futureOriginal, ScaleCropRule scr, ImageBuffer::RecaleMethod method, QString target, const StupidExif & exif) {
  QImage * sced = scaleCropImage(futureOriginal, scr, method);
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
void ImageBuffer::IBData::prepareSC(ScaleCropRule scr, RecaleMethod method) {
  QString scrs = scr.toString();
  qDebug() << "prepare: " << scrs;
  if (rescales.contains(scrs)) return;
  rescales.insert(
    scrs,
    QtConcurrent::run(scaleCropImage, originalImage, scr, method)
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
  qDebug()<<"fileSC '"<<targetFile<<"' "<<scr.toString();
  fileRescaling = QtConcurrent::run(scaleCropToFile, originalImage, scr, ImageBuffer::RLANCZOS4, targetFile, exifData);
}

// check if all the fileSC have finished, wait if not
void ImageBuffer::IBData::waitForFileRescaling() {
  if (fileRescalingRunning) {
    bool stuff = fileRescaling.result();
    stuff = stuff; // we do nothing
    fileRescalingRunning = false;
  }
}

void ImageBuffer::IBData::waitForAllRescaling() {
  foreach (QFuture<QImage *> resc, rescales) {
    volatile QImage * resc_res = resc.result();
    resc_res = resc_res; // -Wunused_variable
  }
  waitForFileRescaling();
}

QSize ImageBuffer::IBData::getOriginalSize() {
  int w, h, temp;
  w = exifData.getTagXResolution();
  h = exifData.getTagYResolution();
  if (exifData.getTagOrientation() == 6 || exifData.getTagOrientation() == 8) {
	  temp = w; w = h; h = temp;
  }
  if (w == 0 || h == 0) {
    w = originalImage.result()->width();
    h = originalImage.result()->height();
  }
  return QSize(w, h);
}
