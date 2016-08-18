#include "AssetPreviewDialog.h"
#include "AssetPreviewPixelData.h"
#include "AssetPreviewMeshData.h"
#include "ui_AssetPreviewDialog.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

using namespace cp::asset;


AssetPreviewDialog::AssetPreviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AssetPreviewDialog)
  , xoffset(0)
{
    ui->setupUi(this);
}

AssetPreviewDialog::~AssetPreviewDialog()
{
//    for( auto it = pixmapItems.begin(), eit = pixmapItems.end(); it != eit; ++it )
//    {
//        delete *it;
//    }

    delete ui;
}

void AssetPreviewDialog::initFromAsset(Asset::Reader reader)
{
    if( reader.hasPixelData() && reader.getPixelData().hasCooked())
    {
        auto viewer = new AssetPreviewPixelData();
        viewer->initFromAsset(reader.getPixelData());
        ui->widget->layout()->addWidget(viewer);
//        ui->verticalLayout_2->addWidget(viewer);
        //ui->frame->layout()->addWidget(viewer);
        return;
    }
    else if( reader.hasMeshData() )
    {
        setVisible(true);
        auto viewer = new AssetPreviewMeshData();
        viewer->initFromAsset(reader.getMeshData());
        //ui->widget->layout()->addWidget(viewer);
        //viewer->setVisible(true);
        QMetaObject::invokeMethod( viewer, "setVisible", Qt::QueuedConnection, Q_ARG( bool, true ) );
        return;
    }


}

