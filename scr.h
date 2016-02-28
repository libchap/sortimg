#ifndef _SCR_H_
#define _SCR_H_


#include <QSize>
#include <QString>
#include <QRect>

#include "colorop.h"

// the information struct
// keeping instructions how the image shall be (in this order):
// 1) rotated
// 2) resized
// 3) cropped
// 4) color-corrected
//
// also some methods easing difficult manipulations, following the user's needs


struct ScaleCropRule {
  double resize_w = 1.0, resize_h = 1.0;
  double crop_x = 0.0, crop_y = 0.0;

  int target_w = 0, target_h = 0;

  int rotate = 0;

  int brightness = +0;

  bool operator==(const ScaleCropRule & s) const;
  bool operator!=(const ScaleCropRule & s) const { return !(*this == s); }

  void rezoom(double scale_zoom, double keepx, double keepy);

  QString toString() const;
  QString toShortString() const;
  QSize scaleSize() const { return QSize(target_w * resize_w, target_h * resize_h); }
  QRect cropRect() const;
  int getTargetPx() const;

  bool isJustResize() const { return (resize_w == 1.0 && resize_h == 1.0 && crop_x == 0.0 && crop_y == 0.0); }
  bool hasTarget() const { return (target_w > 0 && target_h > 0); }
  //bool isNull() const { return ((scale_w == 0 || scale_h == 0 || crop_w == 0 || crop_h == 0) && ini_rot == 0); }

  void retarget(const QSize & targetSize);
  void retarget_to_bound(QSize bound);
  void retarget(int targetPx);

  void rotate_left();
  void rotate_right();
};



#endif // #ifndef _SCR_H_
