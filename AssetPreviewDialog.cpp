#include "AssetPreviewDialog.h"
#include "AssetPreviewMeshData.h"
#include "AssetPreviewPixelData.h"
#include "ui_AssetPreviewDialog.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>

using namespace cp::asset;

AssetPreviewDialog::AssetPreviewDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AssetPreviewDialog)
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
    if (widget_ != nullptr)
    {
        ui->widget->layout()->removeWidget(widget_);
        widget_ = nullptr;
    }
    if (reader.hasPixelData() && reader.getPixelData().hasCooked())
    {
        auto viewer = new AssetPreviewPixelData();
        viewer->initFromAsset(reader.getPixelData());
        ui->widget->layout()->addWidget(viewer);
        widget_ = viewer;
        //        ui->verticalLayout_2->addWidget(viewer);
        // ui->frame->layout()->addWidget(viewer);
        return;
    }
    else if (reader.hasMeshData())
    {
        auto viewer = new AssetPreviewMeshData();
        viewer->initFromAsset(reader.getMeshData());
        viewer->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
        ui->widget->layout()->addWidget(viewer);
        viewer->setVisible(true);
        widget_ = viewer;
        // QMetaObject::invokeMethod( viewer, "setVisible", Qt::QueuedConnection, Q_ARG( bool, true ) );
        return;
    }
}
