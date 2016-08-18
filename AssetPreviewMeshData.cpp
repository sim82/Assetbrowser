#include "AssetPreviewMeshData.h"
#include "SceneMeshDBViewer.h"
#include "ui_AssetPreviewMeshData.h"

AssetPreviewMeshData::AssetPreviewMeshData(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AssetPreviewMeshData)
{
    ui->setupUi(this);
}

AssetPreviewMeshData::~AssetPreviewMeshData()
{
    delete ui;
}

void AssetPreviewMeshData::initFromAsset(cp::asset::AssetMeshData::Reader asset)
{
    ui->openGLWidget->setNavigatable( std::make_unique<SceneMeshDBViewer>(asset));
}
