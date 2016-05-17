#-------------------------------------------------
#
# Project created by QtCreator 2016-05-15T18:15:14
#
#-------------------------------------------------

QT += core xml gui network #dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += qt console warn_on debug
QMAKE_CXXFLAGS += -std=c++11
TARGET = octoxbps-notifier
TEMPLATE = app
DESTDIR += ../bin
OBJECTS_DIR += ../build-octoxbps-notifier
MOC_DIR += ../build-octoxbps-notifier
UI_DIR += ../build-octoxbps-notifier

SOURCES += main.cpp \
    mainwindow.cpp \
    setupdialog.cpp \
    outputdialog.cpp \
    ../../src/terminal.cpp \
    ../../src/unixcommand.cpp \
    ../../src/package.cpp \
    ../../src/wmhelper.cpp \
    ../../src/settingsmanager.cpp \
    #../pacmanhelper/pacmanhelperclient.cpp \
    ../../src/utils.cpp \
    ../../src/transactiondialog.cpp \
    ../../src/argumentlist.cpp \
    ../../src/xbpsexec.cpp \
    ../../src/searchlineedit.cpp \
    ../../src/searchbar.cpp

HEADERS  += \
    mainwindow.h \
    setupdialog.h \
    outputdialog.h \
    ../../src/uihelper.h \
    ../../src/terminal.h \
    ../../src/unixcommand.h \
    ../../src/wmhelper.h \
    ../../src/strconstants.h \
    ../../src/package.h \
    #../pacmanhelper/pacmanhelperclient.h \
    ../../src/utils.h \
    ../../src/transactiondialog.h \
    ../../src/argumentlist.h \
    ../../src/xbpsexec.h \
    ../../src/searchlineedit.h \
    ../../src/searchbar.h

FORMS += ../../ui/transactiondialog.ui \
    ui/setupdialog.ui

RESOURCES += \
    ../../resources.qrc
