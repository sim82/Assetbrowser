#-------------------------------------------------
#
# Project created by QtCreator 2015-03-19T19:04:02
#
#-------------------------------------------------

QT       += core gui quick

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
    scene.capnp.c++ \
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
    MainWindow.cpp \
    AssetPreviewPixelData.cpp \
    AssetPreviewMeshData.cpp \
    MyGLWidget.cpp \
    SceneMeshDBViewer.cpp

HEADERS  += \
    asset.capnp.h \
    scene.capnp.h \
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
    MainWindow.h \
    AssetPreviewPixelData.h \
    AssetPreviewMeshData.h \
    MyGLWidget.h \
    GLNavigatable.h \
    SceneMeshDBViewer.h

FORMS    += \
    AssetProviderDialog.ui \
    AssetBrowserElement.ui \
    AssetPreviewDialog.ui \
    BrowserContent.ui \
    MainWindow.ui \
    AssetPreviewPixelData.ui \
    AssetPreviewMeshData.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    dark.qss
