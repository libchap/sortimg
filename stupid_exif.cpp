#include "stupid_exif.h"
#include <cstring> // memcpy
#include <cstdio> // FIXME: remove !

// sizeof value makes sense to be 2 or 4
// reads from a PBYTE dump a specific-endian-coded value
unsigned long read_endian(PBYTE from, int sizeof_value, ENDIAN endian) {
  int i;
  long val = 0;

  for (i = 0; i < sizeof_value; i++) {
    if (endian == EINTEL) {
      val |= ((unsigned char) from[i]) << (8*i);
    }
    else {
      val <<= 8;
      val |= ((unsigned char) from[i]);
    }
  }
  return val;
}

void write_endian(unsigned long val, PBYTE to, int sizeof_value, ENDIAN endian) {
  for (int i = 0; i < sizeof_value; i++) {
    if (endian == EINTEL) {
      to[i] = val & 0xff;
      val >>= 8;
    }
    else {
      to[i] = (val >> ((sizeof_value - i - 1) * 8)) & 0xff;
    }
  }
}


void stupidexif_error(int code, int hint) {
  qDebug()<<"Warning: StupidExif error "<<code<<" ("<<hint<<")";
}


// read a part of the jpeg
// part of the jpeg is a block starting with jpeg type (e.g. exif = 0xffe1)
// followed with its length
// and followed with (length - 2) bytes of data
// the return value MUST be released by delete [] !!!!! (unless returned error)
BYTE * read_jpeg_part(QDataStream & from, int * part_type, int * part_length) {
  int PT = 0xffff, PL = 0;
  BYTE temp[2];
  PBYTE result = NULL;

  if (from.readRawData(temp, 2) == 2) {
    PT = read_endian(temp, 2, EMOTOROLA);
    //qDebug()<<"ReadJpegPart: "<<QString("%1").arg(PT , 0, 16);
    if (PT != 0xffd8 && PT != 0xffd9 && from.readRawData(temp, 2) == 2) { // has no "else"
      PL = read_endian(temp, 2, EMOTOROLA);
      if (PL > 0 && (result = new BYTE[PL+2]) != NULL) {
	if (from.readRawData(result + 4, PL - 2) == PL - 2) {
	  write_endian(PT, result, 2, EMOTOROLA);
	  write_endian(PL, result + 2, 2, EMOTOROLA);
	  // success
	}
	else {
	  delete [] result;
	  result = NULL;
	  PT = 0xffff; PL = 0;
	}
      }
      else {
	PT = 0xffff; PL = 0;
      }
    }
  }
  if (part_type != NULL) *part_type = PT;
  if (part_length != NULL) * part_length = PL;
  return result;
}


// detect if jpegName points to a valid jpeg
// read whole exif part from it
// tokenize it to find the desired exif tags (orientation, resolution)
// store pointers to those tags' values
void StupidExif::read_from_jpeg(const QString & jpegName) {
  qDebug()<<jpegName;
  QFile jpegFile(jpegName);
  if (!jpegFile.open(QIODevice::ReadOnly)) {
    stupidexif_error(9001, 0);
    return;
  }
  QDataStream jds(&jpegFile);

  int PT, PL;
  BYTE * part;
  long initial_offset_delta;

  read_jpeg_part(jds, &PT, NULL);
  if (PT != 0xffd8) { stupidexif_error(9002, PT); return; }

  while (1) {
    part = read_jpeg_part(jds, &PT, &PL);
    switch (PT) {
      case 0xffd9:
	return; // success ? no, we don't wanna reach the end, rather return at first 0xffe1
      case 0xffff:
	stupidexif_error(9003, PL);
	//qDebug()<<"Maybe: "<<(tagOrientation == NULL ? 0 : 1)<<(tagXResolution == NULL ? 0 : 1)<<(tagYResolution == NULL ? 0 : 1);
	return;
      case 0xffe1:
	this->exifEndian = (read_endian(part + 10, 2, EINTEL) == 0x4949 ? EINTEL : EMOTOROLA);
	if (PL < 16 || read_endian(part + 4, 4, EMOTOROLA) != 0x45786966 ||
	    read_endian(part + 8, 2, EMOTOROLA) != 0 || read_endian(part + 12, 2, exifEndian) != 0x002a) {
	  stupidexif_error(9004, read_endian(part + 12, 2, exifEndian));
          delete [] part;
	}
	else {
	  this->exifData = part;
	  initial_offset_delta = read_endian(part + 14, 4, exifEndian);
	  decodeIFD(part + 18, initial_offset_delta); // success
	  qDebug()<<"Success: "<<(tagOrientation == NULL ? 0 : 1)<<(tagXResolution == NULL ? 0 : 1)<<(tagYResolution == NULL ? 0 : 1);
	}
	return; // instead of break; we don't wanna have multiple exif parts
      default:
	if (part != NULL) delete [] part;
    }
  }
}

// tokenize a single IFD (see jpeg documentation)
void StupidExif::decodeIFD(PBYTE ifd, long offset_delta) {
  int items_count = read_endian(ifd, 2, exifEndian);

  PBYTE ifd_item = ifd + 2;
  int item_tag, item_format;
  long item_data;
  for (int i = 0; i < items_count; i++, ifd_item += 12) {
    item_tag = read_endian(ifd_item, 2, exifEndian);
    item_format = read_endian(ifd_item + 2, 2, exifEndian);
    //item_num_components = read_endian(ifd_item + 4, 4, exifEndian);//useless
    item_data = read_endian(ifd_item + 8, 4, exifEndian);

    if (item_format > 10) {
      stupidexif_error(9101, item_format);
      return;
    }
    // if (item_format == 2) it's a string placed at ifd - offset_delta + item_data
    if (item_tag == 0x8769) {
      decodeIFD(ifd - offset_delta + item_data, offset_delta - item_data);
    }
    if (item_tag == 0x0112) this->tagOrientation = ifd_item + 8;
    if (item_tag == 0xa002) this->tagXResolution = ifd_item + 8;
    if (item_tag == 0xa003) this->tagYResolution = ifd_item + 8;
  }

  long next_ifd = read_endian(ifd_item, 4, exifEndian);
  if (next_ifd != 0) {
    decodeIFD(ifd - offset_delta + next_ifd, next_ifd);
  }
  //qDebug() << "Successful IFD with files: "<<items_count;
}

// check if jpegName points to a valid jpeg
// insert the saved exif part into it (either on the beginning or just after initial JFIF (0xffe0) part if detected)
void StupidExif::insert_into_jpeg(const QString & jpegName) const {
  if (exifData == NULL) return;


  QFile jpegFile(jpegName);
  if (!jpegFile.open(QIODevice::ReadWrite)) {
    stupidexif_error(9201, 0);
    return;
  }
  QDataStream jds(&jpegFile);

  int PT, PL;
  // READING PART we first read the whole jpeg and save all the parts

  read_jpeg_part(jds, &PT, NULL);
  if (PT != 0xffd8) { stupidexif_error(9202, PT); return; }

  PBYTE part;
  PBYTE ffe0_before_ffe1 = NULL;

  part = read_jpeg_part(jds, &PT, &PL);
  if (PT == 0xffe0) {
    ffe0_before_ffe1 = part;
    part = read_jpeg_part(jds, &PT, &PL);
  }

  QByteArray remainder = jpegFile.readAll();


  // WRITING PART we recreate the jpeg using new exif parts and all the saved parts

  jpegFile.seek(0);
  jds << quint16(0xffd8);

  int part_length;

  if (ffe0_before_ffe1 != NULL) {
    part_length = read_endian(ffe0_before_ffe1 + 2, 2, EMOTOROLA);
    if (read_endian(ffe0_before_ffe1, 2, EMOTOROLA) != 0xffe0) { //assert
      stupidexif_error(9205, read_endian(ffe0_before_ffe1, 2, EMOTOROLA));
    }
    qDebug()<<part_length;
    jds.writeRawData(ffe0_before_ffe1, part_length + 2);
  }

  if (read_endian(this->exifData, 2, EMOTOROLA) != 0xffe1) { //assert
    stupidexif_error(9204, read_endian(this->exifData, 2, EMOTOROLA));
  }
  part_length = read_endian(this->exifData + 2, 2, EMOTOROLA);
  qDebug()<<part_length;
  jds.writeRawData(this->exifData, part_length + 2);

  if (PT == 0xffe1 && PL > part_length) {
    jpegFile.write(QByteArray(PL - part_length, '\0'));
  }

  if (PT != 0xffe1) {
    part_length = read_endian(part + 2, 2, EMOTOROLA);
    jds.writeRawData(part, part_length + 2);
  }

  jpegFile.write(remainder);

  jpegFile.close();
}

// interface for manipulation with the important tags stored only as pointers to endian-encoded values in PBYTE array
int StupidExif::getTagOrientation() const {
  return (tagOrientation == NULL ? 0 : read_endian(tagOrientation, 2, exifEndian));
}
void StupidExif::setTagOrientation(int x) {
  if (tagOrientation != NULL) write_endian(x, tagOrientation, 2, exifEndian);
}
int StupidExif::getTagXResolution() const {
  return (tagXResolution == NULL ? 0 : read_endian(tagXResolution, 2, exifEndian));
}
void StupidExif::setTagXResolution(int x) {
  if (tagXResolution != NULL) write_endian(x, tagXResolution, 2, exifEndian);
}
int StupidExif::getTagYResolution() const {
  return (tagYResolution == NULL ? 0 : read_endian(tagYResolution, 2, exifEndian));
}
void StupidExif::setTagYResolution(int x) {
  if (tagYResolution != NULL) write_endian(x, tagYResolution, 2, exifEndian);
}


int StupidExif::getExifDataLen() const {
  return (exifData == NULL ? 0 : read_endian(exifData + 2, 2, exifEndian));
}

// helper function for copy constructor and assignment operators
// clone the parametered instance into this
StupidExif & StupidExif::se_copy_from(const StupidExif & se) {
  exifEndian = se.exifEndian;
  exifData = NULL;
  tagOrientation = NULL;
  tagXResolution = NULL;
  tagYResolution = NULL;
  if (se.exifData != NULL) {
    int edlen = se.getExifDataLen() + 2;
    exifData = new BYTE[edlen];
    if (exifData != NULL) {
      memcpy(exifData, se.exifData, edlen);
      if (se.tagOrientation != NULL) tagOrientation = exifData + (se.tagOrientation - se.exifData);
      if (se.tagXResolution != NULL) tagXResolution = exifData + (se.tagXResolution - se.exifData);
      if (se.tagYResolution != NULL) tagYResolution = exifData + (se.tagYResolution - se.exifData);
    }
  }
  return (*this);
}
