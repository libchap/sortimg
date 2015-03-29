#ifndef _SCR_H_
#define _SCR_H_


#include <QSize>
#include <QString>
#include <QRect>

// the information struct
// keeping instructions how the image shall be (in this order):
// 1) rotated
// 2) resized
// 3) cropped
//
// also some methods easing difficult manipulations, following the user's needs


struct ScaleCropRule {
  int scale_w;
  int scale_h;
  int crop_x;
  int crop_y;
  int crop_w;
  int crop_h;

  int ini_rot;

  ScaleCropRule()
      : scale_w(0), scale_h(0), crop_x(0), crop_y(0), crop_w(0), crop_h(0), ini_rot(0)
      {  }
  ScaleCropRule(int sw, int sh, int cx, int cy, int cw, int ch, int ir = 0)
      : scale_w(sw), scale_h(sh), crop_x(cx), crop_y(cy), crop_w(cw), crop_h(ch), ini_rot(ir)
      {  }
  ScaleCropRule(const QSize & qs, int ir = 0)
      : scale_w(qs.width()), scale_h(qs.height()), crop_x(0), crop_y(0), crop_w(qs.width()), crop_h(qs.height()), ini_rot(ir)
      {  }

  ScaleCropRule rezoom(double scale_zoom, int keepx, int keepy);

  QString toString() const;
  QString toShortString() const;
  QSize scaleSize() { return QSize(scale_w, scale_h); }
  QRect cropRect();

  bool isJustResize() { return (scale_w == crop_w && scale_h == crop_h && crop_x == 0 && crop_y == 0); }

  ScaleCropRule retarget(const ScaleCropRule & targetSize);

  void rotate_left();
  void rotate_right();
};

#endif // #ifndef _SCR_H_
