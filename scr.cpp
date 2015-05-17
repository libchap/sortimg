#include "scr.h"

#include <algorithm> // max, min
using std::min;
using std::max;

// return string representation
// both user readable and unique for each SCR (usable as a map key)
QString ScaleCropRule::toString() const {
  QString res;
  res.sprintf("ScaleCropRule Scale: %dx%d, Crop:%dx%d at %d:%d; rotate=%d", scale_w, scale_h, crop_w, crop_h, crop_x, crop_y, ini_rot);
  return res + co.toString();
}

// short vague user readable description, not unique
QString ScaleCropRule::toShortString() const {
  QString crop;
  if (crop_w > 0 && (scale_w != crop_w || scale_h != crop_h)) {
    crop = "Crop and ";
  }
  QString rot;
  if (ini_rot != 0) {
    rot = "Rotate, ";
  }
  QString res;
  res.sprintf("Rescale to %dpx", max(crop_w, crop_h));
  return rot + crop + res + co.toString();
}

// supply rotate left for the resize and crop information, if rotating left the image
// must keep the resulting size as well as the visible set of pixels after the crop
void ScaleCropRule::rotate_left() {
  int temp;

  temp = scale_w; scale_w = scale_h; scale_h = temp;
  temp = crop_w; crop_w = crop_h; crop_h = temp;
  temp = crop_y;
  crop_y = scale_h - crop_h - crop_x;
  crop_x = temp;

  ini_rot = (ini_rot + 5) % 4;
}
void ScaleCropRule::rotate_right() {
  int temp;

  temp = scale_w; scale_w = scale_h; scale_h = temp;
  temp = crop_w; crop_w = crop_h; crop_h = temp;
  temp = crop_x;
  crop_x = scale_w - crop_w - crop_y;
  crop_y = temp;

  ini_rot = (ini_rot + 3) % 4;
}


// zoom the resize information by the factor scale_zoom,
// move the crop part (keeping crop size) so that the keepx:keepy cropped-part coords
// keep pointing at the same point in the original image
ScaleCropRule ScaleCropRule::rezoom(double scale_zoom, int keepx, int keepy) {
  double new_sw = double(scale_w) * scale_zoom;
  double new_sh = double(scale_h) * scale_zoom;
  double new_cx = double(crop_x + keepx) * scale_zoom;
  double new_cy = double(crop_y + keepy) * scale_zoom;
  //qDebug() << "Rezoom " << scale_zoom << " : " << double(scale_w) << "x" << double(scale_h) << " => " << new_sw << "x" << new_sh;
  return ScaleCropRule(int(new_sw), int(new_sh), int(new_cx) - keepx, int(new_cy) - keepy, crop_w, crop_h, ini_rot, co);
}

// return the crop information as QRect
QRect ScaleCropRule::cropRect() {
  return QRect(crop_x, crop_y, min(scale_w - crop_x, crop_w), min(scale_h - crop_y, crop_h));

}

// change both resize and crop information so that the resulting size of the image will be targetSize
ScaleCropRule ScaleCropRule::retarget(const ScaleCropRule & targetSize) {
  double coef1 = double(targetSize.crop_w) / double(crop_w);
  double coef2 = double(targetSize.crop_h) / double(crop_h);
  double coef = min(coef1, coef2);

  return ScaleCropRule(int(coef * double(scale_w)), int(coef * double(scale_h)),
		       int(coef * double(crop_x)), int(coef * double(crop_y)),
		       int(coef * double(crop_w)), int(coef * double(crop_h)),
		       ini_rot, co);
}
