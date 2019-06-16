TEMPLATE = app
TARGET = lconvert
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
#LIBS += -lexif -lm 
CONFIG += debug
QT += widgets
QT += concurrent

# Input
HEADERS += stupid_exif.h memleak.h colorop.h 
HEADERS += si_globals.h fbiterator.h filebank.h imagebuffer.h scr.h resampler.h 
SOURCES += lconvert.cpp stupid_exif.cpp memleak.cpp colorop.cpp 
SOURCES += fbiterator.cpp filebank.cpp imagebuffer.cpp ibdata.cpp scr.cpp resampler.cpp 

# installation
target.path = /usr/bin
INSTALLS += target

