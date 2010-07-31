#-------------------------------------------------
#
# Project created by QtCreator 2010-06-30T21:54:12
#
#-------------------------------------------------

QT       += core gui

TARGET = Unwrap360
TEMPLATE = app

DESTDIR = build
OBJECTS_DIR = $$DESTDIR
MOC_DIR = $$DESTDIR
RCC_DIR = $$DESTDIR
UI_DIR = $$DESTDIR

INCLUDEPATH += src

profile {
    QMAKE_CFLAGS += -pg
    QMAKE_CXXFLAGS += -pg
    QMAKE_LFLAGS += -pg
}

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/imagearea.cpp \
    src/imagemarker.cpp \
    src/settingsdialog.cpp \
    src/interpolation.cpp

HEADERS  += src/mainwindow.h \
    src/imagearea.h \
    src/fullscreenexitbutton.h \
    src/imagemarker.h \
    src/settingsdialog.h

FORMS    += src/mainwindow.ui \
    src/settingsdialog.ui

CONFIG += mobility
MOBILITY = 

symbian {
    TARGET.UID3 = 0xebcc80b7
    # TARGET.CAPABILITY += 
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}

RESOURCES += \
    data/icons.qrc

OTHER_FILES += \
    data/unwrap360.desktop

#
# Install
#

INSTALLS += target desktop icons48 icons64

isEmpty(PREFIX) {
    PREFIX = /usr
}

BINDIR  = $$PREFIX/bin
DATADIR = $$PREFIX/share

isEmpty(PKGDATADIR) {
    PKGDATADIR = $$DATADIR
}

DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

target.path = $$BINDIR
desktop.path = $$DATADIR/applications/hildon
desktop.files += data/unwrap360.desktop

icons48.path = $$DATADIR/icons/hicolor/48x48/apps
icons48.files += data/icons/48x48/unwrap360.png

icons64.path = $$DATADIR/icons/hicolor/64x64/apps
icons64.files += data/icons/64x64/unwrap360.png

