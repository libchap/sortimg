
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

SortImg::SortImg() : pixmapViewer(QPixmap(":/title.jpg")) {

  setCentralWidget(&pixmapViewer);


  statusBar()->addPermanentWidget(&statusbar, 1);

  pixmapViewer.setMinimumSize(si_settings_initial_width, si_settings_initial_height);
  pixmapViewer.setMaximumSize(si_settings_initial_width, si_settings_initial_height);

  status("<b>SortImg</b> 1.0 &copy; Libcha 2015");
  
  default_vss = 0;
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

  if (renameMode) {
    if (event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z) {
      markRename(event->text());
    }
    renameMode = false;
    return;
  }

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
      if (event->modifiers() & Qt::ShiftModifier) {
	    next_big();
	  }
	  else {
	    next();
	  }
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
    case Qt::Key_G:
      markResize(si_settings_size_G);
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
    case Qt::Key_W:
      if (event->modifiers() & Qt::ShiftModifier) {
	    applyResizeToAll();
	  }
	  else {
	    setResizeAsDefault();
	  }
	  break;
    case Qt::Key_V:
      renameMode = true;
      break;
    case Qt::Key_K: // debug
      status(view_scr.toString());
      break;
    case Qt::Key_L: // debug
      status(main_iterator->getSCR().toString());
      break;
    case Qt::Key_M: // debug
      printAllocs();
      break;
    case Qt::Key_Comma:
      adjustBrightness(+1);
      break;
    case Qt::Key_Period:
      adjustBrightness(-1);
      break;
    case Qt::Key_U:
      adjustGamma(+1);
      break;
    case Qt::Key_I:
      adjustGamma(-1);
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
  if (ibuf == NULL || main_iterator == NULL) return;

  double x = (event->position().x() - pixmapViewer.x()) / (double) pixmapViewer.width();
  double y = (event->position().y() - pixmapViewer.y()) / (double) pixmapViewer.height();

  double mydelta = double(event->angleDelta().y()) / si_settings_mousewheel_step_base;
  double ratio = pow(si_settings_mousewheel_zoom_base, mydelta);
  //qDebug() << "Zooming to ratio " << ratio << " (mydelta " << mydelta << ")";

  view_scr.rezoom(ratio, x, y);
  refreshCurrent();

  ScaleCropRule futurescr(view_scr);
  futurescr.retarget(pixmapViewer.size());
  for (int i = 1; i <= si_settings_preload_zooms; i++) {
    futurescr.rezoom(ratio, x, y);
    ibuf->prepareRescale(view_fname, futurescr);
  }
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
    ibuf->default_scr.retarget(pixmapViewer.size());
  }
}

// call on the beginning or when changed to another source directory
bool SortImg::reInitialize(const QString & path) {
  if (ibuf != NULL) delete ibuf;
  ibuf = new ImageBuffer();
  ibuf->default_scr.retarget(pixmapViewer.size());

  if (fbank != NULL) delete fbank;
  fbank = new FileBank(path);
  if (main_iterator != NULL) delete main_iterator;
  main_iterator = fbank->iterator();
  if (main_iterator->isValid()/*ib->importDirectory(path)*/) {
    ibuf->addRange(main_iterator->subiterator_post(si_settings_preload_images_min));
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


// call when moved to another source photo
void SortImg::viewCurrent() {
  if (ibuf == NULL || fbank == NULL || main_iterator == NULL) {
    pixmapViewer.changePixmap(QPixmap(":/title.jpg"));
    status("<b>SortImg</b> 1.0 &copy; Libcha 2015");
    welcomeShown  = true;
    return;
  }

  view_fname = **main_iterator;
  view_origsize = ibuf->getOriginalSize(view_fname);
  view_scr = (main_iterator)->getSCR();
  view_trash_zoom = view_scr.isJustResize();

  //if (!view_scr.hasTarget())
    view_scr.retarget(view_origsize);

  if (refreshCurrent()) {
	QString autoResize;
    if (default_vss > 0 && !main_iterator->getSCR().hasTarget()) {
		markResize(default_vss);
		autoResize = "Auto Scale ";
	}
	  
    QString deletedMessage = fbank->isMarkedAsDeleted(view_fname) ? " DELETE!" : "";
    QString cnf = QFileInfo(view_fname).baseName();
    status(QString("<b>") + cnf + "</b> (" + size2string(ibuf->getOriginalSize(view_fname)) + ") <b>"
       + autoResize + main_iterator->getSCR().toShortString() + deletedMessage + "</b> ["
	   + QString::number(main_iterator->item_index() + 1) + "/"
	   + QString::number(main_iterator->total_items()) + "]");
  }
}

// call when changed view_scr while staying at the same photo
bool SortImg::refreshCurrent() {
  if (ibuf == NULL || main_iterator == NULL) return 0;
  ScaleCropRule futurescr(view_scr);
  futurescr.retarget_to_bound(pixmapViewer.size());
  QImage * currqi = ibuf->getRescaled(view_fname, futurescr);
  if (currqi == NULL) return 0;
  pixmapViewer.changePixmap(QPixmap::fromImage(*currqi));
  // FIXME : delete currqi ??
  return 1;
}

// got o next image and preload
void SortImg::next() {
  if (fbank == NULL || ibuf == NULL || main_iterator == NULL) return;

  if (!main_iterator->hasNext()) return;

  targetResize();

  ibuf->removeImage(*main_iterator->prev_get(si_settings_preload_images_max));
  main_iterator->next_go();
  //ibuf->addRange(main_iterator->subiterator_post(si_settings_preload_images));
  FBIterator toAdd = main_iterator->next_get(si_settings_preload_images_min);
  ibuf->addImage(*toAdd);

  viewCurrent();
}

void SortImg::next_big() {
  if (fbank == NULL || ibuf == NULL || main_iterator == NULL) return;

  if (!main_iterator->hasNext()) return;

  targetResize();

  do {
    ibuf->removeImage(*main_iterator->prev_get(si_settings_preload_images_max));
    main_iterator->next_go();
    ibuf->addImage(**main_iterator);
  } while (ibuf->getOriginalSize(**main_iterator).height() <= si_settings_big_height);

  ibuf->addRange(main_iterator->subiterator_post(si_settings_preload_images_min));
  viewCurrent();
}

void SortImg::prev() {
  if (fbank == NULL || ibuf == NULL || main_iterator == NULL) return;

  if (!main_iterator->hasPrev()) return;

  targetResize();

  ibuf->removeImage(*main_iterator->next_get(si_settings_preload_images_max));
  main_iterator->prev_go();
  //ibuf->addRange(main_iterator->subiterator_post(si_settings_preload_images));
  FBIterator toAdd = main_iterator->prev_get(si_settings_preload_images_min);
  ibuf->addImage(*toAdd);

  viewCurrent();
}

void SortImg::markResize(int res) {
  view_scr.retarget(res);

  status(QString("<b>Resize to: %1 !</b>").arg(res));
}

void SortImg::markCrop() {
  if (main_iterator == NULL || ibuf == NULL) return;

  if (view_trash_zoom) {
    status(QString("<b>Crop: ") + view_scr.toString() + "</b>");
    view_trash_zoom = 0;
  }
  else {
    status("<b>DO NOT CROP</b>");
    view_trash_zoom = 1;
  }
}

void SortImg::rotateLeft() {
  if (main_iterator == NULL || ibuf == NULL) return;

  view_scr.rotate_left();
  refreshCurrent();
}


void SortImg::rotateRight() {
  if (main_iterator == NULL || ibuf == NULL) return;

  view_scr.rotate_right();
  refreshCurrent();
}

void SortImg::adjustBrightness(int sgn) {
  if (main_iterator == NULL || ibuf == NULL) return;

  int amount = sgn * si_settings_brightness_step;

  view_scr.brightness += amount;
  refreshCurrent();

  ScaleCropRule futurescr(view_scr);
  futurescr.retarget(pixmapViewer.size());
  for (int i = 1; i <= si_settings_preload_brightness; i++) {
    futurescr.brightness += amount;
    ibuf->prepareRescale(view_fname, futurescr);
  }
}

void SortImg::adjustGamma(int sgn) {
  if (main_iterator == NULL || ibuf == NULL) return;

  int amount = sgn * si_settings_gamma_step;

  view_scr.gamma += amount;
  refreshCurrent();

  ScaleCropRule futurescr(view_scr);
  futurescr.retarget(pixmapViewer.size());
  for (int i = 1; i <= si_settings_preload_gamma; i++) {
    futurescr.gamma += amount;
    ibuf->prepareRescale(view_fname, futurescr);
  }
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

void SortImg::markRename(const QString &prefix) {
  if (fbank == NULL || ibuf == NULL || main_iterator == NULL) return;

  QString cn = **main_iterator;

  if (prefix.at(0).isLetter()) {
    status("<b>Rename to prefix " + prefix + " !</b>");
    fbank->markRenamed(cn, prefix);
  }
  else {
    status("<b>Do NOT rename!</b>");
    fbank->markRenamed(cn, "");
  }
}

// call when leaving an image (e.g. moving to another one)
// do the changes done by the user into tmpDir
void SortImg::targetResize() {
  if (fbank == NULL || ibuf == NULL || main_iterator == NULL) return;
  ScaleCropRule oldtarget = main_iterator->getSCR();
  oldtarget.retarget(view_origsize);
  ScaleCropRule newtarget(view_scr);
  if (view_trash_zoom) {
    newtarget.resize_w = 1.0; newtarget.resize_h = 1.0; newtarget.crop_x = 0.0; newtarget.crop_y = 0.0;
  }
  if (newtarget != oldtarget) {
    main_iterator->setSCR(newtarget);
    ibuf->rescaleToFile(view_fname, newtarget, fbank->getTmpNameFor(view_fname));
  }
}


void SortImg::applyResizeToAll() {
  if (fbank == NULL || ibuf == NULL) return;
  int vss = view_scr.getTargetPx();
  if (vss < 1) {
    status(QString("<b>Cannot apply to all current zero resize.</b>"));
    return;
  }
  
  ScaleCropRule targ;
  FBIterator * it = fbank->iterator();
  int i, imax = it->total_items();
  for (i = 0; i < imax; i++, it->next_go()) {
    ibuf->addImage(**it, false);
    targ.retarget(ibuf->getOriginalSize(**it));
    targ.retarget(vss);
    ibuf->rescaleToFile(**it, targ, fbank->getTmpNameFor(**it));
  }
  delete it;
  status(QString("<b>Resized all to %1 !!</b>").arg(vss));
}

void SortImg::setResizeAsDefault() {
  if (fbank == NULL || ibuf == NULL || main_iterator == NULL) return;
  int vss = view_scr.getTargetPx();
  if (vss < 1) {
    status(QString("<b>Cannot set as default current zero resize.</b>"));
    return;
  }
  
  default_vss = vss;
  status(QString("Resize to %1 set as default !").arg(vss));
}


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
