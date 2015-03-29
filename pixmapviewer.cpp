#include "pixmapviewer.h"

PixmapViewer::PixmapViewer(const QPixmap & p) : original_pixmap(p) {
  setAlignment(Qt::AlignCenter);

  update_pixmap();
}

// resize the pixmap to fit the widget if needed (shall be rare)
// actually view the resulting pixmap
void PixmapViewer::update_pixmap() {
  QSize mySize = size(), opSize = original_pixmap.size();
  if (mySize.width() == opSize.width() || mySize.height() == opSize.height()) { // no need to rescale
    rescaled_pixmap = original_pixmap;
  }
  else if (mySize.width() >= opSize.width() && mySize.height() >= opSize.height()) { // keep smaller pixmaps
    rescaled_pixmap = original_pixmap;
  }
  else {
    rescaled_pixmap = original_pixmap.scaled(mySize, Qt::KeepAspectRatio, Qt::FastTransformation);
  }
  setPixmap(rescaled_pixmap);
  setMinimumSize(QSize(10,10));
}

void PixmapViewer::resizeEvent(QResizeEvent* event) {
  QLabel::resizeEvent(event);
  update_pixmap();
}

void PixmapViewer::changePixmap(const QPixmap & p) {
  original_pixmap = p;
  update_pixmap();
  qDebug() << "PixmapViewer pixmap changed (" << size2string(p.size()) << ")";
}
