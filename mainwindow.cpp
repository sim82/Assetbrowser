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
#include "assetcollection.h"
#include "assetcollectionitemmodel.h"
#include "assetproviderserver.h"
#include <assetpreviewdialog.h>

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

    ac = new AssetCollection("/home/sim/src_3dyne/dd_081131_exec/assets/", this);
    ac->fullRescan();

    previewCache = new AssetCollectionPreviewCache( *ac, this );
    previewCache->setObjectName("previewCache");

    // must be called after initilizing previewCache for signal/slot auto-connect.
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
    itemModel = new QStandardItemModel(ui->listView);
    ui->listView->setModel(itemModel);
    ui->listView->setAcceptDrops(true);
    ui->listView->setDragEnabled(true);
    ui->listView->setDropIndicatorShown(true);
    ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

//    connect(itemModel, SIGNAL(itemChanged(QStandardItem*)), ui->listView, SLOT(updatePanel(QStandardItem*)));

    //ui->listView->setItemDelegate(new QStyledItemDelegate());


    ui->listView->setItemDelegate(new ElementViewDelegate(*previewCache));
    ui->listView->setViewMode(QListView::IconMode);
    ui->listView->setResizeMode(QListView::Adjust);
    ui->listView->setIconSize(QSize(128, 128));


    providerServer_ = new AssetProviderServer( *ac, this );
    providerServer_->start();
#if 0
    bundle = std::make_unique<BundleData>();
    capnp::ReaderOptions readerOptions;
    readerOptions.traversalLimitInWords = 1024 * 1024 * 1024;
    capnp::FlatArrayMessageReader reader(kj::ArrayPtr<const capnp::word>((const capnp::word*)bundle->ptr
                                                                         , size_t(bundle->fileSize / sizeof(capnp::word)))
                                         , readerOptions);

    auto bundleReader = reader.getRoot<AssetBundle>();

    capnp::List<Asset>::Reader listReader = bundleReader.getAssets();
    auto numAssets = listReader.size();

    //for( size_t j = 0; j < 1000; ++j )
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
#else

    QIcon defaultIcon(":res/replacement.png");
    auto const& uuids = ac->idList();
    for( size_t i = 0; i < uuids.size(); ++i )
    {

#if 0
        try {
            capnp::FlatArrayMessageReader fr(ac->at(i));
            Asset::Reader assetReader = fr.getRoot<Asset>();


            if( !assetReader.hasPixelData() )
            {
                continue;
            }
            if( !assetReader.getPixelData().hasStored() )
            {
                continue;
            }




            AssetPixelDataStored::Reader storedReader = assetReader.getPixelData().getStored();
            const uchar *data = storedReader.getData().begin();
            const uint len = storedReader.getData().size();

            QPixmap pm;
            pm.loadFromData(data, len, mimetypeToQtImageType(storedReader.getMimeType().begin()));



            QString title = assetReader.getName().cStr();

            QStandardItem *item = new QStandardItem();


            //QImage image(":res/cross.png");
            QImage image(pm.toImage().mirrored());
            item->setData( image, ElementViewDelegate::IconRole);

    //        auto image = QImage(dataReader.begin()
    //                , mipmapLevelReader.getWidth()
    //                , mipmapLevelReader.getHeight()
    //                , toQImageFormat(cookedReader.getPixelFormat()));//.mirrored(false, true);
    //        item->setData( image, ElementViewDelegate::IconRole);
            item->setData( title, ElementViewDelegate::headerTextRole);
            itemModel->appendRow(item);
        }
        catch( kj::Exception x )
        {
            std::cout << "capnp exception: " << x.getDescription().cStr() << "\n";
        }
#else
        QStandardItem *item = new QStandardItem();


        item->setData( uuids[i], ElementViewDelegate::RawDataRole);
        item->setData( defaultIcon, ElementViewDelegate::IconRole);
//        item->setData( title, ElementViewDelegate::headerTextRole);
        idToRowMap.insert(uuids[i], itemModel->rowCount());
        itemModel->appendRow(item);
#endif
    }
#endif
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


    //int iconSize = 32 + ui->horizontalSlider->value() * 16;
    int iconSize = ui->horizontalSlider->value();
    ui->label->setText(QString("size: %1").arg(ui->horizontalSlider->value()));
    ui->listView->setIconSize(QSize(iconSize, iconSize));

    if( bSelection )
    {

        ui->listView->scrollTo(index);
    }
}

void MainWindow::on_listView_activated(const QModelIndex &index)
{

}

void MainWindow::on_listView_doubleClicked(const QModelIndex &index)
{
    QUuid id = index.data(ElementViewDelegate::RawDataRole).toUuid();
    const AssetCollection::Entry &ent = ac->entry(id);

    capnp::ReaderOptions readerOptions;
    //readerOptions.traversalLimitInWords = 1024 * 1024 * 1024;
    capnp::FlatArrayMessageReader fr(kj::ArrayPtr<const capnp::word>((capnp::word const *)ent.mappedData, ent.file.size() / sizeof(capnp::word)));
    Asset::Reader assetReader = fr.getRoot<Asset>();

    AssetPreviewDialog *dialog = new AssetPreviewDialog(this);

    capnp::MallocMessageBuilder builder;
    Asset::Builder assetBuilder = builder.initRoot<Asset>();
    bakeImpl(assetReader, assetBuilder, true);
    dialog->initFromAsset(assetBuilder);




//    if( assetReader.hasPixelData() && assetReader.getPixelData().hasStored())
//    {
//        QImage refImage;
//        capnp::Data::Reader data = assetReader.getPixelData().getStored().getData();
//        refImage.loadFromData(data.begin(), data.size());
//        dialog->initFromImage(refImage);
//    }
    {

        capnp::MallocMessageBuilder builder;
        Asset::Builder assetBuilder = builder.initRoot<Asset>();
        bakeImpl(assetReader, assetBuilder, false);
        dialog->initFromAsset(assetBuilder);
    }
    dialog->setAttribute( Qt::WA_DeleteOnClose, true );
    dialog->setVisible(true);
}

void MainWindow::on_previewCache_previewIconsChanged(QSet<QUuid> ids)
{
    for( auto it = ids.begin(), eit = ids.end(); it != eit; ++it )
    {
//        std::cout << "preview changed: " << (*it).toString().toStdString() << std::endl;
        auto idIt = idToRowMap.find(*it);
        if( idIt == idToRowMap.end() )
        {
            throw std::runtime_error( "no known model row for id");
        }

        QModelIndex index = itemModel->index(idIt.value(), 0);
        itemModel->setData(index, previewCache->get(idIt.key()), ElementViewDelegate::IconRole);


//        itemModel->item(idIt.value(), 0)->setData(previewCache->get(idIt.key()), ElementViewDelegate::IconRole);



    }
}
