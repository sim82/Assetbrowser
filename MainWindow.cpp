#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "AssetBrowserElement.h"
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
#include <QModelIndex>
#include <QScrollBar>
#include <QTimer>
#include <QtQuick/QQuickWindow>
#include "ElementViewDelegate.h"
#include "AssetCollection.h"
#include "AssetCollectionItemModel.h"
#include "AssetCollectionOutlineModel.h"
#include "AssetProviderServer.h"
#include <AssetPreviewDialog.h>

using namespace cp::asset;

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

    collections_.push_back(new AssetCollection("/home/sim/src_3dyne/dd_081131_exec/assets/", this));
    collections_.back()->fullRescan();

    collections_.push_back(new AssetCollection("/tmp/shadermesh_assets/", this));
    collections_.back()->fullRescan();


    previewCache = new AssetCollectionPreviewCache( this );
    for( auto ac : collections_ )
    {
        previewCache->addCollection(ac);
    }
    previewCache->setObjectName("previewCache");

    preloadTimer = new QTimer(this);
    preloadTimer->setObjectName("preloadTimer");

    elementViewDelegate = new ElementViewDelegate();
    elementViewDelegate->setObjectName("elementViewDelegate");
    // FIXME: check why autoconnect does not work
    QObject::connect( elementViewDelegate, SIGNAL(itemPainted(QUuid)), this, SLOT(on_elementViewDelegate_itemPainted(QUuid)));


    // must be called after initilizing previewCache for signal/slot auto-connect.
    ui->setupUi(this);

    outlineModel = new AssetCollectionOutlineModel(ui->treeView);
    for( auto ac : collections_ )
    {
        outlineModel->addCollection(ac);
    }
    ui->treeView->setModel(outlineModel);

    ui->treeView->expandAll();

//    itemModel = new QStandardItemModel(ui->listView);
//    ui->listView->setModel(itemModel);
    ui->listView->setAcceptDrops(true);
    ui->listView->setDragEnabled(true);
    ui->listView->setDropIndicatorShown(true);
    ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    ui->listView->setItemDelegate(elementViewDelegate);
    ui->listView->setViewMode(QListView::IconMode);
    ui->listView->setResizeMode(QListView::Adjust);
    ui->listView->setIconSize(QSize(128, 128));

    QObject::connect(ui->listView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(listviewScrollbar_valueChanged(int)));


    providerServer_ = new AssetProviderServer( *collections_.front(), this );
    providerServer_->start();


    QIcon defaultIcon(":res/replacement.png");


    QVector<QString> prefixList = outlineModel->prefixList();


    for( auto itPrefix = prefixList.begin(), eitPrefix = prefixList.end(); itPrefix != eitPrefix; ++itPrefix )
    {


        QStandardItemModel *itemModel = new QStandardItemModel(ui->listView);

        itemModels.insert(*itPrefix, itemModel);
        currentItemModel = *itPrefix;
        ui->listView->setModel(itemModel);


        for( auto ac : collections_ )
        {
            auto uuids = ac->idListForPrefix(*itPrefix);
            std::cout << "prefix: " << (*itPrefix).toStdString() << " " << uuids.size() << std::endl;

            for( int i = 0; i < uuids.size(); ++i )
            {
                QStandardItem *item = new QStandardItem();
                item->setData( uuids[i], ElementViewDelegate::RawDataRole);
                item->setData( defaultIcon, ElementViewDelegate::IconRole);
                idToRowAndModelMap.insert(uuids[i], qMakePair(itemModel->rowCount(), itemModel));
                itemModel->appendRow(item);
            }
        }
    }


}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_listView_doubleClicked(const QModelIndex &index)
{
    QUuid id = index.data(ElementViewDelegate::RawDataRole).toUuid();

    // HACKHACKHACK!!!
    const AssetCollection::Entry &ent = collections_.back()->entry(id);

    capnp::ReaderOptions readerOptions;
    //readerOptions.traversalLimitInWords = 1024 * 1024 * 1024;
    capnp::FlatArrayMessageReader fr(kj::ArrayPtr<const capnp::word>((capnp::word const *)ent.mappedData, ent.file.size() / sizeof(capnp::word)));
    Asset::Reader assetReader = fr.getRoot<Asset>();



    if( assetReader.hasPixelData() )
    {
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
    else if( assetReader.hasMeshData() )
    {
#if 1
        AssetPreviewDialog *dialog = new AssetPreviewDialog(this);
        dialog->initFromAsset(assetReader);
        dialog->setAttribute( Qt::WA_DeleteOnClose, true );
        dialog->setVisible(true);
#else
        auto qwin = new QQuickWindow();
        qwin->setVisible(true);
#endif

    }

}

void MainWindow::on_previewCache_previewIconsChanged(QSet<QUuid> ids)
{
    for( auto it = ids.begin(), eit = ids.end(); it != eit; ++it )
    {
//        std::cout << "preview changed: " << (*it).toString().toStdString() << std::endl;
        auto idIt = idToRowAndModelMap.find(*it);
        if( idIt == idToRowAndModelMap.end() )
        {
            throw std::runtime_error( "no known model row for id");
        }

        int row = idIt.value().first;
        QStandardItemModel *model = idIt.value().second;
        QModelIndex index = model->index(row, 0);
        model->setData(index, previewCache->get(idIt.key()), ElementViewDelegate::IconRole);


//        itemModel->item(idIt.value(), 0)->setData(previewCache->get(idIt.key()), ElementViewDelegate::IconRole);



    }
}

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{

}

void MainWindow::on_treeView_activated(const QModelIndex &index)
{

}

void MainWindow::on_treeView_pressed(const QModelIndex &index)
{
    if( index.data(Qt::UserRole).isValid() )
    {
        std::cout << "clicked: " << index.data(Qt::UserRole).toString().toStdString() << std::endl;
        currentItemModel = index.data(Qt::UserRole).toString();
        QStandardItemModel *model = itemModels[currentItemModel];

        ui->listView->setModel(model);
    }
}

void MainWindow::on_treeView_entered(const QModelIndex &index)
{

}

void MainWindow::listviewScrollbar_valueChanged(int)
{

}

void MainWindow::on_preloadTimer_timeout()
{
    QStandardItemModel *curModel = itemModels[currentItemModel];

    //QModelIndex first = ui->listView->indexAt(ui->listView->visibleRegion().)

    QRegion viewRect = ui->listView->visibleRegion();

    QMultiMap<int, QUuid> sortedIds;
    for( auto it = preloadSet.begin(), eit = preloadSet.end(); it != eit; ++it )
    {
        auto idIt = idToRowAndModelMap.find(*it);
        if( idIt == idToRowAndModelMap.end() )
        {
            throw std::runtime_error( "no known model row for id");
        }

        QStandardItemModel *model = idIt.value().second;
        if( model != curModel )
        {
            continue;
        }

        int row = idIt.value().first;
        QModelIndex index = model->index(row, 0);

        QRect rect = ui->listView->visualRect(index);


        bool visible = viewRect.intersects(rect);

        if( !visible )
        {
            continue;
        }
        // preview cache loads them in lifo order -> insert bottom to top to make them appear from top to bottom
        sortedIds.insert(-rect.top(), *it);
//        ui->listView
    }


    for( auto it = sortedIds.begin(), eit = sortedIds.end(); it != eit; ++it )
    {
        previewCache->use(it.value());
    }
}

void MainWindow::on_elementViewDelegate_itemPainted(QUuid id)
{
#if 1
    if( preloadTimer->isActive() )
    {
        preloadTimer->stop();
        preloadTimer->setSingleShot(true);
    }
    preloadTimer->start(100);
    preloadSet.insert(id);
#else
    if( !preloadTimer->isActive())
    {
        preloadTimer->setSingleShot(true);
        preloadTimer->start(300);
    }
    preloadSet.insert(id);
#endif
}

