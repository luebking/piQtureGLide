HEADERS = qglimageviewer.h  qgldiashow.h
SOURCES = qglimageviewer.cpp qgldiashow.cpp qgldia-main.cpp
CONFIG += qt
QT		+= opengl gui core
LIBS += -lGLU
DEFINES += VERSION=0.7
system("$QTDIR"/bin/qmake --version 2>&1 | grep " 4." > /dev/null):DEFINES += QT4

