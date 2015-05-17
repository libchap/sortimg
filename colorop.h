#ifndef _COLOROP_H_
#define _COLOROP_H_

#include "si_globals.h"
#include <QVector>
#include <QImage>


class ColorOperation {
  
public:
  ColorOperation() : brightness(0) {  }
  ColorOperation(const ColorOperation & co) { brightness = co.brightness; }
  
  void setBrightness(int new_brightness) { brightness = new_brightness; }
  int getBrightness() const { return brightness; }
  void increaseBrightness(int brightness_step = si_settings_brightness_step) { brightness += brightness_step; }
  void decreaseBrightness(int brightness_step = si_settings_brightness_step) { increaseBrightness(-brightness_step); }
  
  QImage applyOn(const QImage & qi) const;
  
  QString toString() const;
  
private:
  int brightness;
  
  
};









#endif // ifndef _COLOROP_H_
