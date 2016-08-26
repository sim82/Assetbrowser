#ifndef CQT3DASSETVIEWER_H
#define CQT3DASSETVIEWER_H

#include "asset.capnp.h"
#include <QWidget>
#include <QMap>
#include <QVector>
#include <QUuid>

namespace Qt3DExtras
{
class Qt3DWindow;
class QFirstPersonCameraController;
}
namespace Qt3DCore
{
class QEntity;
}
namespace Qt3DRender
{
class QCamera;
}
class CQt3dAssetViewer
        : public QWidget
{
    Q_OBJECT
public:
    CQt3dAssetViewer(QWidget *parent = nullptr);

    void addAsset( cp::asset::Asset::Reader asset );
private:
    Qt3DExtras::Qt3DWindow *view;
    Qt3DCore::QEntity *rootEntity;
    Qt3DRender::QCamera *cameraEntity;
    Qt3DExtras::QFirstPersonCameraController *camController;
    QMap<QUuid, QVector<Qt3DCore::QEntity*>> entities;
};

#endif // CQT3DASSETVIEWER_H
