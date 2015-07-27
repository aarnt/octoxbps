#-------------------------------------------------
#
# Project created by QtCreator 2015-07-26T18:00:00
#
#-------------------------------------------------

QT += core gui network xml dbus widgets quick quickwidgets
DEFINES += OCTOPKG_EXTENSIONS
CONFIG += qt warn_on debug
QMAKE_CXXFLAGS += -std=c++11
TEMPLATE = app
DESTDIR += bin
OBJECTS_DIR += build
MOC_DIR += build
UI_DIR += build

HEADERS += src/QtSolutions/qtsingleapplication.h \
        src/QtSolutions/qtlocalpeer.h \
        src/mainwindow.h \
        src/strconstants.h \
        src/searchlineedit.h \
        src/argumentlist.h \
        src/settingsmanager.h \
        src/uihelper.h \
        src/package.h \
        src/unixcommand.h \
        src/wmhelper.h \
        src/treeviewpackagesitemdelegate.h \
        src/searchbar.h \
        src/transactiondialog.h \
        src/globals.h \
        src/multiselectiondialog.h \
        src/packagerepository.h \
        src/model/packagemodel.h \
        src/ui/octopitabinfo.h \
        src/utils.h \
        src/terminal.h \
        src/terminalselectordialog.h

SOURCES += src/QtSolutions/qtsingleapplication.cpp \
        src/QtSolutions/qtlocalpeer.cpp \
        src/main.cpp\
        src/mainwindow.cpp \
        src/searchlineedit.cpp \
        src/argumentlist.cpp \
        src/settingsmanager.cpp \
        src/package.cpp \
        src/unixcommand.cpp \
        src/wmhelper.cpp \
        src/treeviewpackagesitemdelegate.cpp \
        src/mainwindow_init.cpp \
        src/mainwindow_transaction.cpp \
        src/mainwindow_events.cpp \
        src/mainwindow_help.cpp \
        src/searchbar.cpp \
        src/mainwindow_searchbar.cpp \
        src/transactiondialog.cpp \
        src/mainwindow_news.cpp \
        src/mainwindow_refresh.cpp \
        src/globals.cpp \
        src/multiselectiondialog.cpp \
        src/packagerepository.cpp \
        src/model/packagemodel.cpp \
        src/ui/octopitabinfo.cpp \
        src/utils.cpp \
        src/terminal.cpp \
        src/terminalselectordialog.cpp

FORMS   += ui/mainwindow.ui \
        ui/transactiondialog.ui \
        ui/multiselectiondialog.ui

RESOURCES += resources.qrc

#TRANSLATIONS += resources/translations/octoxbps_pt_BR.ts
