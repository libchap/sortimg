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
#include <QTreeView>

//#include "global.h"
#include "pixmapviewer.h"
//#include "imagebank.h"
#include "scr.h"
#include "filebank.h"
#include "imagebuffer.h"
#include "si_globals.h"
#include "memleak.h"
#include "colorop.h"

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

  void disputeDefaultSCR(const FBIterator & accordingTo);

  void viewCurrent();
  bool refreshCurrent();
  void next();
  void next_big();
  void prev();

  int targetSize = 0;
  void markResize(int res);
  void targetResize();
  void markDelete();
  void markCrop();
  void markRename(const QString &prefix);

  void rotateLeft();
  void rotateRight();
  void adjustBrightness(int sgn = +1);
  void adjustGamma(int sgn = +1);

  void finalizeResize();
  
  void setResizeAsDefault();
  void applyResizeToAll();

  QString viewDirectoryDialog();

  PixmapViewer pixmapViewer;

  //ImageBank * ib = NULL;
  FileBank * fbank = NULL;
  FBIterator * main_iterator = NULL;
  ImageBuffer * ibuf = NULL;

  ScaleCropRule view_scr;
  QString view_fname;
  QSize view_origsize;
  bool view_trash_zoom;
  int default_vss;


private:

  QLabel statusbar;
  bool welcomeShown = true;
  bool renameMode = false;

};

#endif // #ifndef _SORTIMG_H_
