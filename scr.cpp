#include "scr.h"

#include <algorithm> // max, min, swap
using std::min;
using std::max;
using std::swap;

// return string representation
// both user readable and unique for each SCR (usable as a map key)
QString ScaleCropRule::toString() const {
  QString res;
  if (hasTarget()) res.sprintf("ScaleCropRule Rotate=%d; Scale: %dx%d, Crop %dx%d at %d:%d; Brightness=%+d%%; Gamma=%d",
                               rotate, int (target_w * resize_w), int(target_h * resize_h), target_w, target_h,
                               int(target_w * crop_x), int(target_h * crop_y), brightness, gamma);
  else res.sprintf("ScaleCropRule just Rotate=%d; Brightness=%+d; Gamma=%d", rotate, brightness, gamma);
  return res;
}

bool ScaleCropRule::operator==(const ScaleCropRule & s) const {
  return (resize_w == s.resize_w && resize_h == s.resize_w && crop_x == s.crop_x &&
          crop_y == s.crop_y && target_w == s.target_w && target_h == s.target_h &&
          rotate == s.rotate && brightness == s.brightness && gamma == s.gamma);
}

// short vague user readable description, not unique
QString ScaleCropRule::toShortString() const {
  QString res;
  if (rotate != 0) res += "Rotate, ";
  if (hasTarget()) {
    res += "Scale ";
    if (!isJustResize()) res += "and Crop ";
  }
  if (brightness != 0) res += "and Color correction";
  if (gamma != 100) res += " and Gamma correction";
  return res;
}

// supply rotate left for the resize and crop information, if rotating left the image
// must keep the resulting size as well as the visible set of pixels after the crop
void ScaleCropRule::rotate_left() {
  rotate = (rotate + 5) % 4;

  swap(target_w, target_h);
  swap(resize_w, resize_h);
  swap(crop_x, crop_y);
  crop_y = resize_h - 1.0 - crop_y;
}


void ScaleCropRule::rotate_right() {
  rotate = (rotate + 3) % 4;

  swap(target_w, target_h);
  swap(resize_w, resize_h);
  swap(crop_x, crop_y);
  crop_x = resize_w - 1.0 - crop_x;
}


// zoom the resize information by the factor scale_zoom,
// move the crop part (keeping crop size) so that the keepx:keepy cropped-part coords
// keep pointing at the same point in the original image
void ScaleCropRule::rezoom(double scale_zoom, double keepx, double keepy) {
  resize_w *= scale_zoom;
  resize_h *= scale_zoom;
  crop_x = (crop_x + keepx) * scale_zoom - keepx;
  crop_y = (crop_y + keepy) * scale_zoom - keepy;
}

// return the crop information as QRect
QRect ScaleCropRule::cropRect() const {
  return QRect(max(crop_x * target_w, 0.0), max(crop_y * target_w, 0.0), target_w * min(resize_w - crop_x, 1.0), target_h * min(resize_h - crop_y, 1.0));

}

void ScaleCropRule::retarget(const QSize & targetSize) {
  target_w = targetSize.width();
  target_h = targetSize.height();
}

void ScaleCropRule::retarget_to_bound(QSize bound) {
  if (!hasTarget()) return;
  double aspect = target_w / (double) target_h;
  double bound_aspect = bound.width() / (double) bound.height();
  if (aspect > bound_aspect) bound.setHeight(bound.width() / aspect);
  else bound.setWidth(bound.height() * aspect);
  retarget(bound);
}

// WARNING this is something different than retarget(const QSize & targetSize) !!
void ScaleCropRule::retarget(int targetPx) {
  retarget_to_bound(QSize(targetPx, targetPx));
}

int ScaleCropRule::getTargetPx() const {
  return max(target_w, target_h);
}
