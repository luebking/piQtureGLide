HEADERS = qglimageviewer.h qgliv.h dirfilecombo.h
SOURCES = qglimageviewer.cpp qgliv.cpp main.cpp dirfilecombo.cpp ui.cpp
CONFIG += qt debug
QT += opengl gui core concurrent
LIBS += -lGLU
DEFINES += VERSION=0.9
target.path += $$[QT_INSTALL_BINS]
INSTALLS += target

!mac {
unix {
    DATADIR = $$[QT_INSTALL_PREFIX]/share
    DEFINES += "DATADIR=$$DATADIR"
    INSTALLS += desktop icon shader postinstall

    desktop.path = $$DATADIR/applications
    desktop.files += piQtureGLide.desktop

    icon.path = $$DATADIR/icons/hicolor/128x128/apps
    icon.files += piQtureGLide.png

    shader.path = $$DATADIR/piQtureGLide
    shader.files += shader.vert shader.frag

    postinstall.path =  $$[QT_INSTALL_BINS]
    postinstall.extra = update-desktop-database
}
}
