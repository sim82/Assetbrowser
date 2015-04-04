#-------------------------------------------------
#
# Project created by QtCreator 2015-03-19T19:04:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AssetBrowser
LIBS += -lcapnp -lcapnp-rpc -lkj -lkj-async
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++14
QMAKE_EXT_CPP += .c++
SOURCES += main.cpp\
        mainwindow.cpp\
        asset.capnp.c++ \
    assetbrowserelement.cpp \
    browsercontent.cpp \
    flowlayout.cpp \
    elementviewdelegate.cpp \
    assetcollection.cpp \
    assetcollectionitemmodel.cpp \
    assetcollectionpreviewcache.cpp \
    assetproviderserver.cpp \
    assetpreviewdialog.cpp

HEADERS  += mainwindow.h \
    assetbrowserelement.h \
    browsercontent.h \
    flowlayout.h \
    elementviewdelegate.h \
    assetcollection.h \
    assetcollectionitemmodel.h \
    assetcollectionpreviewcache.h \
    assetproviderserver.h \
    assetpreviewdialog.h

FORMS    += mainwindow.ui \
    assetbrowserelement.ui \
    browsercontent.ui \
    assetpreviewdialog.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    dark.qss
