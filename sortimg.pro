TEMPLATE = app
TARGET = sortimg
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
#LIBS += -lexif -lm 
CONFIG += debug

# Input
HEADERS += sortimg.h pixmapviewer.h stupid_exif.h memleak.h colorop.h 
HEADERS += si_globals.h fbiterator.h filebank.h imagebuffer.h scr.h resampler.h 
SOURCES += sortimg.cpp pixmapviewer.cpp stupid_exif.cpp memleak.cpp colorop.cpp 
SOURCES += fbiterator.cpp filebank.cpp imagebuffer.cpp ibdata.cpp scr.cpp resampler.cpp 
RESOURCES += sortimg.qrc



# installation
target.path = /usr/bin
INSTALLS += target

