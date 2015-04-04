#include "assetpreviewdialog.h"
#include "ui_assetpreviewdialog.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

enum class pixel_format_type : uint32_t {
    RGBA,
    RGB,
    BGR,
    Luminance
};

static QImage::Format toQImageFormat( uint32_t f )
{
    switch( pixel_format_type(f) )
    {
    case pixel_format_type::RGB:
    case pixel_format_type::BGR:
    default:
        return QImage::Format_RGB888;

    case pixel_format_type::RGBA:
        return QImage::Format_RGBA8888;

    case pixel_format_type::Luminance:
        return QImage::Format_Indexed8;
    }
}

AssetPreviewDialog::AssetPreviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AssetPreviewDialog)
  , xoffset(0)
{
    ui->setupUi(this);

    scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);
}

AssetPreviewDialog::~AssetPreviewDialog()
{
    delete ui;
}

void AssetPreviewDialog::initFromAsset(Asset::Reader reader)
{
    if( !reader.hasPixelData() || !reader.getPixelData().hasCooked())
    {
        return;
    }

    AssetPixelDataCooked::Reader cookedReader = reader.getPixelData().getCooked();

    capnp::List<MipMapLevel>::Reader levelList = cookedReader.getLevels();
    capnp::List<capnp::Data>::Reader dataList = cookedReader.getLevelData();


    qreal yoffset = 0;
    for( int i = 0; i < levelList.size(); ++i )
    {
        MipMapLevel::Reader lr = levelList[i];
        capnp::Data::Reader dr = dataList[i];

        QImage image((const uchar*)dr.begin(), lr.getWidth(), lr.getHeight(), toQImageFormat(cookedReader.getPixelFormat()));

        QGraphicsPixmapItem *item = scene->addPixmap(QPixmap::fromImage(image));
        item->setOffset(xoffset,yoffset);

        yoffset += image.height();
    }

    xoffset += levelList[0].getWidth();
}

void AssetPreviewDialog::on_horizontalSlider_sliderMoved(int position)
{
    QMatrix matrix;
    matrix.scale(position/100.0, position/100.0);
    ui->graphicsView->setMatrix(matrix);
}
