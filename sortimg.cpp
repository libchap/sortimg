
#include <QApplication>
#include <QStatusBar>

#include <cmath> // pow()

#include "sortimg.h"


int main(int argc, char ** argv) {
  Q_INIT_RESOURCE(sortimg);

  QApplication app(argc, argv);
  app.setApplicationName("SortImg");

  SortImg mainwindow;
  mainwindow.show();

  return app.exec();
}

QString size2string(const QSize & s) {
  if (s.isValid()) {
    QString str;
    return str.sprintf("%dx%d", s.width(), s.height());
  }
  else return "InvalidSize";
}

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



SortImg::SortImg() : pixmapViewer(QPixmap(":/title.jpg")) {

  setCentralWidget(&pixmapViewer);


  statusBar()->addPermanentWidget(&statusbar, 1);

  pixmapViewer.setMinimumSize(si_settings_initial_width, si_settings_initial_height);
  pixmapViewer.setMaximumSize(si_settings_initial_width, si_settings_initial_height);

  status("<b>SortImg</b> 1.0 &copy; Libcha 2015");
}

SortImg::~SortImg() {
  //ib->exportToCfgFile();
  delete ibuf;
  delete fbank;
  delete main_iterator;
}

void SortImg::keyPressEvent(QKeyEvent * event) {
  QString tmp;

  QMainWindow::keyPressEvent(event);

  // switch 1, see below switch 2
  switch (event->key()) {
    case Qt::Key_Escape:
    case Qt::Key_Q:
      QApplication::quit();
      break;
    case Qt::Key_S:
      if(event->modifiers() & Qt::ShiftModifier) {
        if (!(tmp = viewDirectoryDialog()).isEmpty()) {
          welcomeShown = false;
          reInitialize(tmp);
        }
      }
      break;
    case Qt::Key_Shift:
      return;
    default:
      break;
  }

  if (welcomeShown) {
    //if (reInitialize(".")) {
    welcomeShown = false;
    reInitialize(".");
    return;
    //}
  }

  // switch 2, see switch 1 above
  switch (event->key()) {
    case Qt::Key_Right:
      next();
      break;
    case Qt::Key_Left:
      prev();
      break;
    case Qt::Key_R:
      if (event->modifiers() & Qt::ShiftModifier) {
	    finalizeResize();
      }
      break;
    case Qt::Key_A:
      markResize(si_settings_size_A);
      break;
    case Qt::Key_B:
      markResize(si_settings_size_B);
      break;
    case Qt::Key_C:
      markResize(si_settings_size_C);
      break;
    case Qt::Key_D:
      markResize(si_settings_size_D);
      break;
    case Qt::Key_E:
      markResize(si_settings_size_E);
      break;
    case Qt::Key_F:
      markResize(si_settings_size_F);
      break;
    case Qt::Key_X:
      markCrop();
      break;
    case Qt::Key_O:
      rotateLeft();
      break;
    case Qt::Key_P:
      rotateRight();
      break;
    case Qt::Key_K: // debug
      status(scr.toString());
      break;
    case Qt::Key_L: // debug
      status(main_iterator->getSCR().toString());
      break;
    case Qt::Key_Delete:
      markDelete();
      break;
    default:
      break;
  }
}

void SortImg::wheelEvent(QWheelEvent * event) {
  QMainWindow::wheelEvent(event);
  if (ibuf == NULL) return;

  int x = event->x() - pixmapViewer.x();
  int y = event->y() - pixmapViewer.y();

  double mydelta = double(event->delta()) / si_settings_mousewheel_step_base;
  double ratio = pow(si_settings_mousewheel_zoom_base, mydelta);
  double ratio_pre = ratio;
  //qDebug() << "Zooming to ratio " << ratio << " (mydelta " << mydelta << ")";

  QString cn = **main_iterator;

  for (int i = 1; i <= si_settings_preload_zooms; i++, ratio_pre *= ratio) {
    ibuf->prepareRescale(cn, scr.rezoom(ratio_pre, x, y));
  }
  //qDebug() << "Pre-rezoom: " << scr.toString();
  scr = scr.rezoom(ratio, x, y);
  //qDebug() << "Post-rezoom: " << scr.toString();
  QImage * rqi = ibuf->getRescaled(cn, scr);
  pixmapViewer.changePixmap(QPixmap::fromImage(*rqi));
}


void SortImg::showEvent(QShowEvent * event) {
  QMainWindow::showEvent(event);
  // do nothing, we wait for enter after welcome screen :)
  move(si_settings_initial_x, si_settings_initial_y);
  qDebug() << "PW size: " << size2string(pixmapViewer.size());
  qDebug() << "PW min: " << size2string(pixmapViewer.minimumSize());
  qDebug() << "PW max: " << size2string(pixmapViewer.maximumSize());
}
void SortImg::resizeEvent(QResizeEvent* event) {
  QMainWindow::resizeEvent(event);
  if (ibuf != NULL) {
    ibuf->default_scr = ScaleCropRule(pixmapViewer.size());
  }
}

bool SortImg::reInitialize(const QString & path) {
  if (ibuf != NULL) delete ibuf;
  ibuf = new ImageBuffer();
  ibuf->default_scr = ScaleCropRule(pixmapViewer.size());

  if (fbank != NULL) delete fbank;
  fbank = new FileBank(path);
  if (main_iterator != NULL) delete main_iterator;
  main_iterator = fbank->iterator();
  if (main_iterator->isValid()/*ib->importDirectory(path)*/) {
    ibuf->addRange(main_iterator->subiterator_post(si_settings_preload_images));
    viewCurrent();
  }
  else {
    QMessageBox::warning(this, "SortImg", QString("No JPG files found in path: ") + path);
    delete ibuf; ibuf = NULL;
    delete fbank; fbank = NULL;
    delete main_iterator; main_iterator = NULL;
  }
  viewCurrent();
  return (ibuf != NULL);
}

void SortImg::finalizeResize() {
  if (fbank == NULL) return;

  targetResize();
  ibuf->waitForFileRescales();

  fbank->finalizeTmpDir();
}

void SortImg::viewCurrent() {
  if (ibuf == NULL || fbank == NULL || main_iterator == NULL) {
    pixmapViewer.changePixmap(QPixmap(":/title.jpg"));
    status("<b>SortImg</b> 1.0 &copy; Libcha 2015");
    welcomeShown  = true;
    return;
  }

  scr = ibuf->default_scr;

  QString cn = **main_iterator;
  QImage * currqi = ibuf->getRescaled(cn);
  QString deletedMessage = fbank->isMarkedAsDeleted(cn) ? " DELETE!" : "";
  if (currqi != NULL) {
    pixmapViewer.changePixmap(QPixmap::fromImage(*currqi));
    QString cnf = QFileInfo(cn).baseName();
    status(QString("<b>") + cnf + "</b> (" + size2string(ibuf->getOriginalSize(cn)) + ") <b>"
           + main_iterator->getSCR().toShortString() + deletedMessage + "</b> ["
	   + QString::number(main_iterator->item_index() + 1) + "/"
	   + QString::number(main_iterator->total_items()) + "]");
  }
}

void SortImg::next() {
  if (fbank == NULL || ibuf == NULL || main_iterator == NULL) return;

  if (!main_iterator->hasNext()) return;

  targetResize();

  ibuf->removeImage(*main_iterator->prev_get(si_settings_preload_images));
  main_iterator->next_go();
  //ibuf->addRange(main_iterator->subiterator_post(si_settings_preload_images));
  ibuf->addImage(*main_iterator->next_get(si_settings_preload_images));

  viewCurrent();
}

void SortImg::prev() {
  if (fbank == NULL || ibuf == NULL || main_iterator == NULL) return;

  if (!main_iterator->hasPrev()) return;

  targetResize();

  ibuf->removeImage(*main_iterator->next_get(si_settings_preload_images));
  main_iterator->prev_go();
  //ibuf->addRange(main_iterator->subiterator_post(si_settings_preload_images));
  ibuf->addImage(*main_iterator->prev_get(si_settings_preload_images));

  viewCurrent();
}

void SortImg::markResize(int res) {
  //targetSize = res;

  ScaleCropRule old_scr = main_iterator->getSCR();

  if (old_scr.isJustResize()) {
    main_iterator->setSCR(ScaleCropRule(QSize(res, res)));
  }
  else {
    main_iterator->setSCR(old_scr.retarget(ScaleCropRule(QSize(res, res))));
  }

  status(QString("<b>Resize to: %1 !</b>").arg(res));
}

void SortImg::markCrop() {
  if (main_iterator == NULL || ibuf == NULL) return;

  ScaleCropRule old_scr = main_iterator->getSCR();
  if (old_scr.crop_w == 0) {
    old_scr = ScaleCropRule(ibuf->getOriginalSize(**main_iterator));
  }

  if (old_scr.isJustResize()) {
    ScaleCropRule new_scr = scr.retarget(old_scr);
    main_iterator->setSCR(new_scr);
    status(QString("<b>Crop: ") + new_scr.toString() + "</b>");
  }
  else {
    main_iterator->setSCR(ScaleCropRule(old_scr.cropRect().size()));
    status("<b>DO NOT CROP</b>");
  }
}

void SortImg::rotateLeft() {
  if (main_iterator == NULL || ibuf == NULL) return;

  ScaleCropRule miscr = main_iterator->getSCR();
  miscr.rotate_left();
  main_iterator->setSCR(miscr);

  QString cn = **main_iterator;
  scr.rotate_left();
  //qDebug() << "Post-rezoom: " << scr.toString();
  QImage * rqi = ibuf->getRescaled(cn, scr);
  pixmapViewer.changePixmap(QPixmap::fromImage(*rqi));
}

void SortImg::rotateRight() {
  if (main_iterator == NULL || ibuf == NULL) return;

  ScaleCropRule miscr = main_iterator->getSCR();
  miscr.rotate_right();
  main_iterator->setSCR(miscr);

  QString cn = **main_iterator;
  scr.rotate_right();
  //qDebug() << "Post-rezoom: " << scr.toString();
  QImage * rqi = ibuf->getRescaled(cn, scr);
  pixmapViewer.changePixmap(QPixmap::fromImage(*rqi));
}

void SortImg::markDelete() {
  if (fbank == NULL || ibuf == NULL || main_iterator == NULL) return;

  QString cn = **main_iterator;
  if (fbank->isMarkedAsDeleted(cn)) {
    fbank->unmarkDeleted(cn);
    status("<b>Do NOT delete!</b>");
  }
  else {
    fbank->markAsDeleted(cn);
    status("<b>DELETE !!</b>");
  }
}

void SortImg::targetResize() {
  if (fbank == NULL || ibuf == NULL || main_iterator == NULL) return;
  const ScaleCropRule & targetSCR = main_iterator->getSCR();
  if (targetSCR.crop_w > 0) {
    QString cn = **main_iterator;
    //ScaleCropRule sc(targetSize, targetSize, 0, 0, targetSize, targetSize);
    ibuf->rescaleToFile(cn, targetSCR, fbank->getTmpNameFor(cn));

    targetSize = 0;
  }
}

//void SortImg::msgbox(const QString & msg) {
//  QMessageBox msgBox;
//  msgBox.setText(msg);
//  msgBox.setWindowTitle("SortImg");
//  msgBox.exec();
// STUB

QString SortImg::viewDirectoryDialog() {
  QFileDialog *fd = new QFileDialog;
  QTreeView *tree = fd->findChild <QTreeView*>();
  tree->setRootIsDecorated(true);
  tree->setItemsExpandable(true);
  fd->setFileMode(QFileDialog::Directory);
  //fd->setOption(QFileDialog::ShowDirsOnly);
  fd->setViewMode(QFileDialog::Detail);
  int result = fd->exec();
  if (result) {
    return fd->selectedFiles()[0];
  }
  else {
    return QString();
  }
}
