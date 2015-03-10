TEMPLATE = app
TARGET = sortimg
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
#LIBS += -lexif -lm 
CONFIG += debug

# Input
HEADERS += sortimg.h pixmapviewer.h 
HEADERS += qtopia_exif/qexifimageheader.h qtopia_exif/qtopiaglobal.h 
HEADERS += si_globals.h fbiterator.h filebank.h imagebuffer.h scr.h 
SOURCES += sortimg.cpp pixmapviewer.cpp 
SOURCES += qtopia_exif/qexifimageheader.cpp 
SOURCES += fbiterator.cpp filebank.cpp imagebuffer.cpp ibdata.cpp scr.cpp 
RESOURCES += sortimg.qrc



# installation
target.path = /usr/bin
INSTALLS += target

