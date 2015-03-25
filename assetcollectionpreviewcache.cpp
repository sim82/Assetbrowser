#include "assetcollectionpreviewcache.h"
#include "assetcollection.h"
#include <QSize>
#include <capnp/serialize.h>

AssetCollectionPreviewCache::AssetCollectionPreviewCache(AssetCollection & collection, QObject *parent)
    : QObject(parent)
    , collection_(collection)
{

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
    pixmap.loadFromData(data, len, mimetypeToQtImageType(storedReader.getMimeType().begin()));

    QSize destSize = fitSize(pixmap.size(), QSize(64, 64));

    cache_.emplace( id, QIcon(pixmap.scaled(destSize.width(), destSize.height())));


    return true;
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

