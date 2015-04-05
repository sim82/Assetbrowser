#include "assetcollectionpreviewcache.h"
#include "assetcollection.h"
#include <QSize>
#include <capnp/serialize.h>
#include <QTimer>
#include <QDateTime>
#include <iostream>

AssetCollectionPreviewCache::AssetCollectionPreviewCache(AssetCollection & collection, QObject *parent)
    : QObject(parent)
    , collection_(collection)
    , previewIcon( ":res/cross.png")
    , timer(new QTimer(this))
{
    timer->setObjectName("timer");
    QMetaObject::connectSlotsByName(this);

    timer->setInterval(100);
    timer->setSingleShot(false);
    timer->start();


}

AssetCollectionPreviewCache::~AssetCollectionPreviewCache()
{

}

static QSize fitSize( const QSize &src, const QSize &dest )
{
    if( src.width() >= src.height() )
    {
        return QSize(dest.width(), src.height() * (float(dest.width()) / src.width()));
    }
    else
    {
        return QSize(src.width() * (float(dest.height()) / src.height()), dest.height());
    }
}

static const char *mimetypeToQtImageType(const char *mimetype)
{
    std::string s(mimetype);

    if( s == "image/png")
    {
        return "PNG";
    }
    else if( s == "image/jpeg" )
    {
        return "JPEG";
    }

    return nullptr;
}


bool AssetCollectionPreviewCache::request(const QUuid &id)
{
    auto it = cache_.find(id);

    if( it != cache_.end() )
    {
        return true;
    }
#if 1
    auto const & ent = collection_.entry(id);

    capnp::FlatArrayMessageReader fr(kj::ArrayPtr<const capnp::word>((capnp::word const *)ent.mappedData, ent.file.size() / sizeof(capnp::word)));
    Asset::Reader assetReader = fr.getRoot<Asset>();


    if( !assetReader.hasPixelData() )
    {
        return false;
    }
    if( !assetReader.getPixelData().hasStored() )
    {
        return false;
    }


    AssetPixelDataStored::Reader storedReader = assetReader.getPixelData().getStored();
    const uchar *data = storedReader.getData().begin();
    const uint len = storedReader.getData().size();

    QPixmap pixmap;
    pixmap.loadFromData(data, len/*, mimetypeToQtImageType(storedReader.getMimeType().begin())*/);

    QSize destSize = fitSize(pixmap.size(), QSize(64, 64));

    cache_.emplace( id, QIcon(pixmap.scaled(destSize.width(), destSize.height())));


    return true;
#else
    cache_.emplace( id, previewIcon );
    return true;
#endif
}

QIcon &AssetCollectionPreviewCache::get(const QUuid &id)
{
    auto it = cache_.find(id);

    if( it == cache_.end() )
    {
        throw std::runtime_error( "cache miss" );
    }


    return it->second;
}

void AssetCollectionPreviewCache::use(const QUuid &id)
{
    if( cache_.find(id) != cache_.end() )
    {
        return;
    }

    lifoQueue_.push(id);

    if( !timer->isActive())
    {
        std::cout << "start load timer" << std::endl;
        timer->start();
    }

}

void AssetCollectionPreviewCache::on_timer_timeout()
{
    qint64 start = QDateTime::currentMSecsSinceEpoch();

    QSet<QUuid> dirtyIdSet;

    while( !lifoQueue_.empty() && QDateTime::currentMSecsSinceEpoch() - start < 10 )
    {
        QUuid id = lifoQueue_.top();
        lifoQueue_.pop();

        //std::cout << "lifo pop: " << id.toString().toStdString() << std::endl;

        if( cache_.find(id) != cache_.end() )
        {
            continue;
        }

        auto const & ent = collection_.entry(id);

        capnp::FlatArrayMessageReader fr(kj::ArrayPtr<const capnp::word>((capnp::word const *)ent.mappedData, ent.file.size() / sizeof(capnp::word)));
        Asset::Reader assetReader = fr.getRoot<Asset>();


        if( !assetReader.hasPixelData() || !assetReader.getPixelData().hasStored())
        {
            continue;
        }


        AssetPixelDataStored::Reader storedReader = assetReader.getPixelData().getStored();
        const uchar *data = storedReader.getData().begin();
        const uint len = storedReader.getData().size();

        QPixmap pixmap;
        pixmap.loadFromData(data, len);

        QSize destSize = fitSize(pixmap.size(), QSize(64, 64));

        cache_.emplace( id, QIcon(pixmap.scaled(destSize.width(), destSize.height())));

        dirtyIdSet.insert(id);
    }

    if( lifoQueue_.empty())
    {
        std::cout << "stop load timer" << std::endl;
        timer->stop();
    }
    emit previewIconsChanged(dirtyIdSet);
}

