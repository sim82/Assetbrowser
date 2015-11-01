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
#QMAKE_CXXFLAGS += -std=c++14 -fsanitize=address
#QMAKE_LFLAGS += -fsanitize=address

QMAKE_CXXFLAGS += -std=c++14
QMAKE_EXT_CPP += .c++
SOURCES +=\
        asset.capnp.c++ \
    AssetProviderDialog.cpp \
    AssetProviderServer.cpp \
    AssetBrowserElement.cpp \
    AssetCollection.cpp \
    AssetCollectionItemModel.cpp \
    AssetCollectionOutlineModel.cpp \
    AssetCollectionPreviewCache.cpp \
    AssetPreviewDialog.cpp \
    BrowserContent.cpp \
    ElementViewDelegate.cpp \
    FlowLayout.cpp \
    Main.cpp \
    MainWindow.cpp

HEADERS  += \
    asset.capnp.h \
    AssetProviderDialog.h \
    AssetProviderServer.h \
    AssetBrowserElement.h \
    AssetCollection.h \
    AssetCollectionItemModel.h \
    AssetCollectionOutlineModel.h \
    AssetCollectionPreviewCache.h \
    AssetPreviewDialog.h \
    BrowserContent.h \
    ElementViewDelegate.h \
    FlowLayout.h \
    MainWindow.h

FORMS    += \
    AssetProviderDialog.ui \
    AssetBrowserElement.ui \
    AssetPreviewDialog.ui \
    BrowserContent.ui \
    MainWindow.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    dark.qss
