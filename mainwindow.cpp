#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "assetbrowserelement.h"
#include <QFile>
#include "capnp/serialize.h"
#include "asset.capnp.h"
#include <iostream>
#include <QMessageBox>
#include <QLabel>
#include "GL/gl.h"
#include <cstdint>

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //flowLayout = new FlowLayout(ui->scrollArea);
    //ui->scrollArea->setWidget(flowLayout->widget());
    ui->scrollArea->setWidgetResizable(true);
    flowLayout = new FlowLayout;
    ui->scrollArea->widget()->setLayout(flowLayout);
    ui->scrollArea->setVisible(false);

    QFile f("/home/sim/src_3dyne/dd_081131_exec/out.bundle");

    if( !f.exists())
    {
        QMessageBox::critical( this, "cannot open file", "cannot open file");
    }
    f.open(QIODevice::ReadOnly);
    auto fileSize = f.size();

    std::cout << "file size: " << fileSize << "\n";

    auto * ptr = f.map(0, fileSize);
    capnp::ReaderOptions readerOptions;
    readerOptions.traversalLimitInWords = 1024 * 1024 * 1024;
    capnp::FlatArrayMessageReader reader(kj::ArrayPtr<const capnp::word>((const capnp::word*)ptr, size_t(fileSize / sizeof(capnp::word))), readerOptions);

    AssetBundle::Reader bundleReader = reader.getRoot<AssetBundle>();


    capnp::List<Asset>::Reader listReader = bundleReader.getAssets();
    auto numAssets = listReader.size();

   // for( size_t j = 0; j < 1000; ++j )
    {
        for( size_t i = 0; i < numAssets; ++i )
        {

            Asset::Reader assetReader = listReader[i];

            if( !assetReader.hasPixelData() )
            {
                continue;
            }
            if( !assetReader.getPixelData().hasCooked() )
            {
                continue;
            }

            AssetPixelDataCooked::Reader cookedReader = assetReader.getPixelData().getCooked();

            if( cookedReader.getLevels().size() < 1 )
            {
                continue;
            }

            MipMapLevel::Reader mipmapLevelReader = cookedReader.getLevels()[0];
            capnp::Data::Reader dataReader = cookedReader.getLevelData()[0];


            QString title = assetReader.getName().cStr();


#if 0
            auto * ele = new AssetBrowserElement();
            ele->setTitle(title);
            ele->setFromQImage(QImage(dataReader.begin(), mipmapLevelReader.getWidth(), mipmapLevelReader.getHeight(), toQImageFormat(cookedReader.getPixelFormat())).mirrored(false, true));

            flowLayout->addWidget(ele);

            ele->setVisible(true);
#else
            ui->listWidget->setIconSize(QSize(32,32));
            ui->listWidget->setResizeMode(QListWidget::Adjust);
            ui->listWidget->setViewMode(QListWidget::IconMode);
            ui->listWidget->setMovement(QListWidget::Static);
            ui->listWidget->setTextElideMode(Qt::ElideLeft);
            ui->listWidget->addItem(new QListWidgetItem(QIcon(QPixmap::fromImage(QImage(dataReader.begin(), mipmapLevelReader.getWidth(), mipmapLevelReader.getHeight(), toQImageFormat(cookedReader.getPixelFormat())).mirrored(false, true))), title));
#endif
//            if( qrand() / double(RAND_MAX) < 0.3 )
//            {
//                auto * ele = new AssetBrowserElement();
//                ele->setTitle("meeeep");

//                flowLayout->addWidget(ele);

//                ele->setVisible(true);

//            }

        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
