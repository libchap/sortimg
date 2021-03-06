#ifndef _COLOROP_H_
#define _COLOROP_H_

#include "si_globals.h"
#include <QImage>

QImage changeBrightness( const QImage& image, int brightness );
QImage changeGamma( const QImage& image, int gamma );

#endif // ifndef _COLOROP_H_
