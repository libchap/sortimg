#ifndef _PIXMAPVIEWER_H_
#define _PIXMAPVIEWER_H_

#include <QtGui>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

#include "si_globals.h"

// a widget for viewing a pixmap
// resizing it to fit the widget size of needed
// center the image

// FIXME public -> protected ?
class PixmapViewer : public QLabel {
Q_OBJECT

public:
  PixmapViewer(const QPixmap & p /* initial picture */);
  void changePixmap(const QPixmap & p);

public slots:


  void resizeEvent(QResizeEvent* event); // override, event

  //QSize minimumSize() { return QSize(1024,768); } // override, QWidget

private:


protected:
  QPixmap original_pixmap;
  QPixmap rescaled_pixmap;
  void update_pixmap();
};




#endif // #ifndef _PIXMAPVIEWER_H_
