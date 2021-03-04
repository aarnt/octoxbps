#-------------------------------------------------
#
# Project created by QtCreator 2016-05-15T18:15:14
#
#-------------------------------------------------

QT += core xml gui network widgets
CONFIG += qt console warn_on debug lrelease embed_translations
QM_FILES_RESOURCE_PREFIX=/translations
QMAKE_CXXFLAGS += -std=c++11
TARGET = octoxbps-notifier
TEMPLATE = app
DESTDIR += ../bin
OBJECTS_DIR += build
MOC_DIR += build
UI_DIR += build
LIBS += -lqtermwidget5

SOURCES += main.cpp \
    mainwindow.cpp \
    optionsdialog.cpp \
    outputdialog.cpp \
    ../src/terminal.cpp \
    ../src/termwidget.cpp \
    ../src/unixcommand.cpp \
    ../src/strconstants.cpp \
    ../src/package.cpp \
    ../src/wmhelper.cpp \
    ../src/settingsmanager.cpp \
    ../src/utils.cpp \
    ../src/transactiondialog.cpp \
    ../src/argumentlist.cpp \
    ../src/xbpsexec.cpp \
    ../src/searchlineedit.cpp \
    ../src/searchbar.cpp

HEADERS  += \
    mainwindow.h \
    optionsdialog.h \
    outputdialog.h \
    ../src/uihelper.h \
    ../src/terminal.h \
    ../src/termwidget.h \
    ../src/unixcommand.h \
    ../src/wmhelper.h \
    ../src/strconstants.h \
    ../src/package.h \
    ../src/utils.h \
    ../src/transactiondialog.h \
    ../src/argumentlist.h \
    ../src/xbpsexec.h \
    ../src/searchlineedit.h \
    ../src/searchbar.h

FORMS += ../ui/transactiondialog.ui \
  ui/optionsdialog.ui

RESOURCES += \
    ../resources.qrc

TRANSLATIONS += ../translations/octoxbps_de_DE.ts

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

isEmpty(ETCDIR) {
    ETCDIR = /etc
}

target.path = $$BINDIR

polkit.path = $$DATADIR/polkit-1/actions
polkit.files += polkit/org.freedesktop.policykit.pkexec.run-xbpsinstall.policy
polkit_rules.path = $$ETCDIR/polkit-1/rules.d
polkit_rules.files += polkit/49-nopasswd_limited_xbps_install_sync.rules

desktop.path = $$DATADIR/applications
desktop.files += octoxbps-notifier.desktop

INSTALLS += target polkit polkit_rules desktop
