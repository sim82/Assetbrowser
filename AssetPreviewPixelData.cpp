#include "AssetPreviewPixelData.h"
#include "ui_AssetPreviewPixelData.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

using namespace cp::asset;

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

static int pixelSize( uint32_t f )
{
    switch( pixel_format_type(f) )
    {
    case pixel_format_type::RGB:
    case pixel_format_type::BGR:
    default:
        return 3;

    case pixel_format_type::RGBA:
        return 4;

    case pixel_format_type::Luminance:
        return 1;
    }
}


void AssetPreviewPixelData::initFromAsset(AssetPixelData::Reader reader)
{

    AssetPixelDataCooked::Reader cookedReader = reader.getCooked();

    capnp::List<MipMapLevel>::Reader levelList = cookedReader.getLevels();
    capnp::List<capnp::Data>::Reader dataList = cookedReader.getLevelData();


    qreal yoffset = 0;
    for( size_t i = 0; i < levelList.size(); ++i )
    {
        MipMapLevel::Reader lr = levelList[i];
        capnp::Data::Reader dr = dataList[i];


        QImage image(lr.getWidth(), lr.getHeight(), toQImageFormat(cookedReader.getPixelFormat()));

        int lineSize = pixelSize(cookedReader.getPixelFormat()) * lr.getWidth();
        auto inIt = dr.begin();

        // QImage likes it scanlines to be 32byte aligned... copy in data manually.
        for( size_t h = 0; h < lr.getHeight(); ++h )
        {
            uchar *scanline = image.scanLine(h);

            std::copy( inIt, inIt + lineSize, scanline);
            inIt += lineSize;
        }

//        QImage image((const uchar*)dr.begin(), lr.getWidth(), lr.getHeight(), toQImageFormat(cookedReader.getPixelFormat()));

        QGraphicsPixmapItem *item = scene->addPixmap(QPixmap::fromImage(image));
        item->setOffset(xoffset,yoffset);

        yoffset += image.height() + 16;
    }

    xoffset += levelList[0].getWidth();
}

void AssetPreviewPixelData::initFromImage(const QImage &image)
{
    qreal yoffset = 0;
    QSize size = image.size();
    while( size.width() > 4 && size.height() > 4 )
    {
        QImage outImage = image.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(image.format());

        QGraphicsPixmapItem *item = scene->addPixmap(QPixmap::fromImage(outImage));
        item->setOffset(xoffset,yoffset);

        yoffset += outImage.height();

        size /= 2;
    }
    xoffset += image.width();
}

void AssetPreviewPixelData::on_horizontalSlider_sliderMoved(int position)
{
    QMatrix matrix;
    matrix.scale(position/100.0, position/100.0);
    ui->graphicsView->setMatrix(matrix);
}




AssetPreviewPixelData::AssetPreviewPixelData(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AssetPreviewPixelData)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
}

AssetPreviewPixelData::~AssetPreviewPixelData()
{
    delete ui;
}
