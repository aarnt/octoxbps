#-------------------------------------------------
#
# Project created by QtCreator 2015-07-26T18:00:00
#
#-------------------------------------------------

QT += core gui network xml widgets core5compat
DEFINES += OCTOXBPS_EXTENSIONS UNIFIED_SEARCH
CONFIG += qt warn_on debug lrelease embed_translations
QM_FILES_RESOURCE_PREFIX=/translations
QMAKE_CXXFLAGS += -std=c++17
TEMPLATE = app
DESTDIR += bin
OBJECTS_DIR += build
MOC_DIR += build
UI_DIR += build
LIBS += -lqtermwidget6

HEADERS += src/QtSolutions/qtsingleapplication.h \
        src/QtSolutions/qtlocalpeer.h \
        src/mainwindow.h \
        src/strconstants.h \
        src/searchlineedit.h \
        src/argumentlist.h \
        src/settingsmanager.h \
        src/ui/octoxbpstabinfo.h \
        src/uihelper.h \
        src/package.h \
        src/packagetreeview.h \
        src/unixcommand.h \
        src/wmhelper.h \
        src/treeviewpackagesitemdelegate.h \
        src/searchbar.h \
        src/transactiondialog.h \
        src/globals.h \
        src/packagerepository.h \
        src/model/packagemodel.h \
        src/utils.h \
        src/terminal.h \
        src/constants.h \
        src/xbpsexec.h \
        src/termwidget.h

SOURCES += src/QtSolutions/qtsingleapplication.cpp \
        src/QtSolutions/qtlocalpeer.cpp \
        src/main.cpp\
        src/mainwindow.cpp \
        src/strconstants.cpp \
        src/searchlineedit.cpp \
        src/argumentlist.cpp \
        src/settingsmanager.cpp \
        src/package.cpp \
        src/packagetreeview.cpp \
        src/ui/octoxbpstabinfo.cpp \
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
        src/packagerepository.cpp \
        src/model/packagemodel.cpp \
        src/utils.cpp \
        src/terminal.cpp \
        src/xbpsexec.cpp \
        src/termwidget.cpp

FORMS   += ui/mainwindow.ui \
        ui/transactiondialog.ui

RESOURCES += resources.qrc

TRANSLATIONS += translations/octoxbps_de.ts \
             translations/octoxbps_nl.ts \
             translations/octoxbps_ru.ts \
             translations/octoxbps_tr.ts

# install
isEmpty(PREFIX) {
    PREFIX = /usr
}

isEmpty(BINDIR) {
    BINDIR = $$PREFIX/bin
}

isEmpty(DATADIR) {
    DATADIR = $$PREFIX/share
}

target.path = $$BINDIR

desktop.path = $$DATADIR/applications
desktop.files += octoxbps.desktop

icon.path = $$DATADIR/icons/hicolor/48x48/apps
icon.files += resources/images/octopi.png

INSTALLS += target desktop icon
