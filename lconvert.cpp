#include <cstdlib>
#include <cstdio>

#include "imagebuffer.h"

int main(int argc, char ** argv) {
  long dst_size = argc > 1 ? atol(argv[1]) : -1;
  if (argc != 4 || dst_size <= 0) {
      printf("usage: %s <size> <src> <dst>\n", argv[0]);
      return 1;
  }
  
  QString srcFile = argv[2];
  QString targetFile = argv[3];
  
  ImageBuffer ib;
  ib.addImage(srcFile, false);
  
  ScaleCropRule scr;
  scr.retarget(ib.getOriginalSize(srcFile));
  scr.retarget(dst_size);
  
  ib.rescaleToFile(srcFile, scr, targetFile);
  ib.waitForFileRescales();
  
  return 0;
}
