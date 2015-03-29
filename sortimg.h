#ifndef _SORTIMG_H_
#define _SORTIMG_H_

#include <QtGui>
#include <QMainWindow>
#include <QPixmap>
#include <QLabel>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QMessageBox>
#include <QFileDialog>

//#include "global.h"
#include "pixmapviewer.h"
//#include "imagebank.h"
#include "scr.h"
#include "filebank.h"
#include "imagebuffer.h"
#include "si_globals.h"

// central class:
// user interface
// putting together FileBank and ImageBuffer

class SortImg : public QMainWindow {
Q_OBJECT

public:
  SortImg();
  ~SortImg();

  void status(const QString & s) { statusbar.setText(s); }

public slots:
  void showEvent(QShowEvent * event);
  void keyPressEvent(QKeyEvent * event);
  void wheelEvent(QWheelEvent * event);
  void resizeEvent(QResizeEvent* event);

protected:

  bool reInitialize(const QString & path);

  void viewCurrent();
  void next();
  void prev();

  int targetSize = 0;
  void markResize(int res);
  void targetResize();
  void markDelete();
  void markCrop();

  void rotateLeft();
  void rotateRight();

  void finalizeResize();

  QString viewDirectoryDialog();

  PixmapViewer pixmapViewer;

  //ImageBank * ib = NULL;
  FileBank * fbank = NULL;
  FBIterator * main_iterator = NULL;
  ImageBuffer * ibuf = NULL;

  ScaleCropRule scr; // this is the currently viewed scr

private:

  QLabel statusbar;
  bool welcomeShown = true;

};

#endif // #ifndef _SORTIMG_H_
