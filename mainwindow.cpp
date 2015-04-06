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
#include <QFileSystemModel>
#include "elementviewdelegate.h"
#include "assetcollection.h"
#include "assetcollectionitemmodel.h"
#include "assetcollectionoutlinemodel.h"
#include "assetproviderserver.h"
#include <assetpreviewdialog.h>

enum class pixel_format_type : uint32_t {
    RGBA,
    RGB,
    BGR,
    Luminance
};

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

    outlineModel = new AssetCollectionOutlineModel(ui->treeView);
    outlineModel->addCollection(ac);
    ui->treeView->setModel(outlineModel);

    ui->treeView->expandAll();

    itemModel = new QStandardItemModel(ui->listView);
    ui->listView->setModel(itemModel);
    ui->listView->setAcceptDrops(true);
    ui->listView->setDragEnabled(true);
    ui->listView->setDropIndicatorShown(true);
    ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    ui->listView->setItemDelegate(new ElementViewDelegate(*previewCache));
    ui->listView->setViewMode(QListView::IconMode);
    ui->listView->setResizeMode(QListView::Adjust);
    ui->listView->setIconSize(QSize(128, 128));


    providerServer_ = new AssetProviderServer( *ac, this );
    providerServer_->start();


    QIcon defaultIcon(":res/replacement.png");
    auto const& uuids = ac->idList();
    for( size_t i = 0; i < uuids.size(); ++i )
    {
        QStandardItem *item = new QStandardItem();


        item->setData( uuids[i], ElementViewDelegate::RawDataRole);
        item->setData( defaultIcon, ElementViewDelegate::IconRole);
        idToRowMap.insert(uuids[i], itemModel->rowCount());
        itemModel->appendRow(item);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
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
