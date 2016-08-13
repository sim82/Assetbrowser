#include "AssetCollectionPreviewCache.h"
#include "AssetCollection.h"
#include <QSize>
#include <capnp/serialize.h>
#include <QTimer>
#include <QDateTime>
#include <algorithm>
#include <iostream>
#include <cassert>

using namespace cp::asset;

AssetCollectionPreviewCache::AssetCollectionPreviewCache(QObject *parent)
    : QObject(parent)
    , previewIcon( ":res/cross.png")
    , noPreviewIcon( ":res/cross.png")
    , timer(new QTimer(this))
{
    timer->setObjectName("timer");
    QMetaObject::connectSlotsByName(this);

    timer->setSingleShot(false);
    timer->start();


}

AssetCollectionPreviewCache::~AssetCollectionPreviewCache()
{

}

void AssetCollectionPreviewCache::addCollection(AssetCollection *ac)
{
    if( std::find(collections_.begin(), collections_.end(), ac ) != collections_.end() )
    {
        throw std::runtime_error( "collection already member of cache" );
    }

    auto const& ids = ac->idList();

    for( auto const& id : ids )
    {
        auto bRes = idToCollection_.emplace( id, ac ).second;
        assert( bRes );
    }
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

const AssetCollection::Entry &AssetCollectionPreviewCache::findEntry(QUuid const& id )
{
    auto it = idToCollection_.find(id);
    if( it == idToCollection_.end() )
    {
        throw std::runtime_error( "unknown id" );
    }

    auto const& collection = it->second;
    return collection->entry(id);
}

bool AssetCollectionPreviewCache::request(const QUuid &id)
{
    {
        auto it = cache_.find(id);

        if( it != cache_.end() )
        {
            return true;
        }
    }
#if 1

    auto const & ent = findEntry(id);

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
        timer->start(0);
    }

}

void AssetCollectionPreviewCache::on_timer_timeout()
{
    timer->setInterval(100);
    qint64 start = QDateTime::currentMSecsSinceEpoch();

    QSet<QUuid> dirtyIdSet;

    while( !lifoQueue_.empty() && QDateTime::currentMSecsSinceEpoch() - start < 20 )
    {
        QUuid id = lifoQueue_.top();
        lifoQueue_.pop();

        //std::cout << "lifo pop: " << id.toString().toStdString() << std::endl;

        if( cache_.find(id) != cache_.end() )
        {
            continue;
        }

        auto const & ent = findEntry(id);

        capnp::FlatArrayMessageReader fr(kj::ArrayPtr<const capnp::word>((capnp::word const *)ent.mappedData, ent.file.size() / sizeof(capnp::word)));
        Asset::Reader assetReader = fr.getRoot<Asset>();


        if( !assetReader.hasPixelData() || !assetReader.getPixelData().hasStored())
        {
            cache_.emplace( id, QIcon(noPreviewIcon));


            dirtyIdSet.insert(id);
            continue;
        }


        AssetPixelDataStored::Reader storedReader = assetReader.getPixelData().getStored();
        const uchar *data = storedReader.getData().begin();
        const uint len = storedReader.getData().size();


        QImage image;
        image.loadFromData(data, len);

        QSize destSize = fitSize(image.size(), QSize(64, 64));


        cache_.emplace( id, QIcon(QPixmap::fromImage(image.scaled(destSize.width(), destSize.height()))));


        dirtyIdSet.insert(id);
    }

    if( lifoQueue_.empty())
    {
        std::cout << "stop load timer" << std::endl;
        timer->stop();
    }
    emit previewIconsChanged(dirtyIdSet);
}

