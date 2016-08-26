#include "CQt3dAssetViewer.h"
#include "CSceneAttributeArrayInterleavedRenderer.h"

#include <QGuiApplication>

#include <Qt3DCore/qentity.h>
#include <Qt3DRender/qcamera.h>
#include <Qt3DRender/qcameralens.h>

#include <QtGui/QScreen>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QCommandLinkButton>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

#include <Qt3DInput/QInputAspect>

#include <Qt3DExtras/qtorusmesh.h>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/qeffect.h>
#include <Qt3DRender/qmaterial.h>
#include <Qt3DRender/qmesh.h>
#include <Qt3DRender/qrenderpass.h>
#include <Qt3DRender/qsceneloader.h>
#include <Qt3DRender/qtechnique.h>
#include <Qt3DRender/qtexture.h>

#include <Qt3DCore/qaspectengine.h>
#include <Qt3DCore/qtransform.h>

#include <Qt3DExtras/qforwardrenderer.h>
#include <Qt3DRender/qrenderaspect.h>

#include "scene.capnp.h"
#include <Qt3DExtras/qfirstpersoncameracontroller.h>
#include <Qt3DExtras/qt3dwindow.h>

CQt3dAssetViewer::CQt3dAssetViewer(QWidget *parent)
    : QWidget(parent)
{
    view = new Qt3DExtras::Qt3DWindow();
    view->defaultFramegraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    QWidget *container = QWidget::createWindowContainer(view);
    QSize screenSize   = view->screen()->size();
    container->setMinimumSize(QSize(200, 100));
    container->setMaximumSize(screenSize);

    // QWidget *widget      = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setAlignment(Qt::AlignTop);
    hLayout->addWidget(container, 1);
    hLayout->addLayout(vLayout);

    // setWindowTitle(QStringLiteral("Basic shapes"));

    Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;
    view->registerAspect(input);

    // Root entity
    rootEntity = new Qt3DCore::QEntity();

    // Camera
    cameraEntity = view->camera();

    cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0, 0, 20.0f));
    cameraEntity->setUpVector(QVector3D(0, 1, 0));
    cameraEntity->setViewCenter(QVector3D(0, 0, 0));

    // For camera controls
    camController = new Qt3DExtras::QFirstPersonCameraController(rootEntity);
    camController->setCamera(cameraEntity);

    // Scenemodifier
    // SceneModifier *modifier = new SceneModifier(rootEntity, aas);

    // Set root object of the scene
    view->setRootEntity(rootEntity);

}

void CQt3dAssetViewer::addAsset(cp::asset::Asset::Reader asset)
{
    if (!asset.hasMeshData() || !asset.getMeshData().hasAttributeArrayInterleavedList() ||
        asset.getMeshData().getAttributeArrayInterleavedList().size() == 0 )
    {
        return;
    }
    QUuid id(asset.getHeader().getUuid().cStr());
    auto it = entities.find(id);
    if (it != entities.end())
    {
        return;
    }

    for( auto attributeArray : asset.getMeshData().getAttributeArrayInterleavedList() )
    {
        auto geometryRender = new CSceneAttributeArrayInterleavedRenderer(attributeArray);

        // ConeMesh Transform
        auto transform = new Qt3DCore::QTransform();
        // coneTransform->setScale(0.05f);
        // coneTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 45.0f));
        // coneTransform->setTranslation(QVector3D(26.0f, 20.0f, -18.0));

        auto material = new Qt3DExtras::QPhongMaterial();
        material->setDiffuse(QColor(QRgb(0x928327)));

        auto entity = new Qt3DCore::QEntity(rootEntity);
        entity->addComponent(geometryRender);
        entity->addComponent(material);
        entity->addComponent(transform);
    }
}
