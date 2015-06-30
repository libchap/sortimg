TEMPLATE = app
TARGET = sortimg
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
#LIBS += -lexif -lm 
CONFIG += debug
QT += widgets
QT += concurrent

# Input
HEADERS += sortimg.h pixmapviewer.h stupid_exif.h memleak.h colorop.h 
#HEADERS += qtopia_exif/qexifimageheader.h qtopia_exif/qtopiaglobal.h 
HEADERS += si_globals.h fbiterator.h filebank.h imagebuffer.h scr.h 
SOURCES += sortimg.cpp pixmapviewer.cpp stupid_exif.cpp memleak.cpp colorop.cpp 
#SOURCES += qtopia_exif/qexifimageheader.cpp 
SOURCES += fbiterator.cpp filebank.cpp imagebuffer.cpp ibdata.cpp scr.cpp 
RESOURCES += sortimg.qrc



# installation
target.path = /usr/bin
INSTALLS += target

