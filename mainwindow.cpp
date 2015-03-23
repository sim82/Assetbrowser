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
#include <QStyledItemDelegate>
#include <QStandardItemModel>
#include "elementviewdelegate.h"

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

class BundleData {
public:
    BundleData()
        : f("/home/sim/src_3dyne/dd_081131_exec/out.bundle")
    {
        if( !f.exists())
        {
            QMessageBox::critical( nullptr, "cannot open file", "cannot open file");
        }
        f.open(QIODevice::ReadOnly);
        fileSize = f.size();

        std::cout << "file size: " << fileSize << "\n";

        ptr = f.map(0, fileSize);


    }

    const uchar *ptr;
    size_t fileSize;
private:
    QFile f;


};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //flowLayout = new FlowLayout(ui->scrollArea);
    //ui->scrollArea->setWidget(flowLayout->widget());
#if 0
    ui->scrollArea->setWidgetResizable(true);
    flowLayout = new FlowLayout;
    ui->scrollArea->widget()->setLayout(flowLayout);
    ui->scrollArea->setVisible(false);
    ui->listWidget->setVisible(false);
#endif
    itemModel = new QStandardItemModel();
    ui->listView->setModel(itemModel);
    //ui->listView->setItemDelegate(new QStyledItemDelegate());
    ui->listView->setItemDelegate(new ElementViewDelegate());
    ui->listView->setViewMode(QListView::IconMode);
    ui->listView->setResizeMode(QListView::Adjust);
    ui->listView->setIconSize(QSize(128, 128));

    bundle = std::make_unique<BundleData>();
    capnp::ReaderOptions readerOptions;
    readerOptions.traversalLimitInWords = 1024 * 1024 * 1024;
    capnp::FlatArrayMessageReader reader(kj::ArrayPtr<const capnp::word>((const capnp::word*)bundle->ptr
                                                                         , size_t(bundle->fileSize / sizeof(capnp::word)))
                                         , readerOptions);

    auto bundleReader = reader.getRoot<AssetBundle>();

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
            const capnp::Data::Reader dataReader = cookedReader.getLevelData()[0];


            QString title = assetReader.getName().cStr();


#if 0
            auto * ele = new AssetBrowserElement();
            ele->setTitle(title);
            ele->setFromQImage(QImage(dataReader.begin(), mipmapLevelReader.getWidth(), mipmapLevelReader.getHeight(), toQImageFormat(cookedReader.getPixelFormat())).mirrored(false, true));

            flowLayout->addWidget(ele);

            ele->setVisible(true);
#elseif 0
            ui->listWidget->setIconSize(QSize(32,32));
            ui->listWidget->setResizeMode(QListWidget::Adjust);
            ui->listWidget->setViewMode(QListWidget::IconMode);
            ui->listWidget->setMovement(QListWidget::Static);
            ui->listWidget->setTextElideMode(Qt::ElideLeft);
            ui->listWidget->addItem(new QListWidgetItem(QIcon(QPixmap::fromImage(QImage(dataReader.begin(), mipmapLevelReader.getWidth(), mipmapLevelReader.getHeight(), toQImageFormat(cookedReader.getPixelFormat())).mirrored(false, true))), title));
#else
            QStandardItem *item = new QStandardItem();
            //item->setData(title,ListviewDelegate::);

            //item->setIcon();
            //item->setData(QIcon(QPixmap::fromImage(QImage(dataReader.begin(), mipmapLevelReader.getWidth(), mipmapLevelReader.getHeight(), toQImageFormat(cookedReader.getPixelFormat())).mirrored(false, true))), ElementViewDelegate::IconRole);

            auto image = QImage(dataReader.begin()
                    , mipmapLevelReader.getWidth()
                    , mipmapLevelReader.getHeight()
                    , toQImageFormat(cookedReader.getPixelFormat()));//.mirrored(false, true);
#if 0
            item->setData( QPixmap::fromImage(image.mirrored())
                            , ElementViewDelegate::IconRole);
#else
            item->setData( image, ElementViewDelegate::IconRole);
#endif
            item->setData(title, ElementViewDelegate::headerTextRole);
            itemModel->appendRow(item);
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

void MainWindow::on_horizontalSlider_actionTriggered(int action)
{
  //  auto index = ui->listView->selectedIndexes();

    const auto &list = ui->listView->selectionModel()->selectedIndexes();
    bool bSelection = !list.isEmpty();
    QModelIndex index;
    if( bSelection )
    {
        index = list.first();
    }


    int iconSize = 32 + ui->horizontalSlider->value() * 16;
    ui->listView->setIconSize(QSize(iconSize, iconSize));

    if( bSelection )
    {

        ui->listView->scrollTo(index);
    }
}
