#include <QDirIterator>
#include "AssetCollection.h"
#include <iostream>
#include "asset.capnp.h"
#include "capnp/serialize.h"
#include <QVector>
#include <QUuid>
#include <QDateTime>
#include <QSet>
#include <QMultiMap>
#include <cassert>

using namespace cp::asset;

AssetCollection::AssetCollection(const char *path, QObject *parent)
    : QObject(parent), baseDir_(path)
{

    if( !baseDir_.isReadable() )
    {
        throw std::runtime_error( "baseDir not readable" );
    }
}

AssetCollection::~AssetCollection()
{

}

size_t AssetCollection::size() const
{
    return id_asset_map_.size();
}

//AssetCollection::Entry const & AssetCollection::entryAt(int pos) const
//{
//    return *(assets_.at(pos));
//}

const AssetCollection::Entry &AssetCollection::entry(const QUuid &id)
{
    auto it = id_asset_map_.find(id);

    if( it == id_asset_map_.end() )
    {
        throw std::runtime_error( "unknown id" );
    }

    Entry *ent = (it->second).get();

    cacheIn(ent);

    return *ent;
}

std::vector<QUuid> AssetCollection::idList() const
{
    std::vector<QUuid> list;
    list.reserve(id_asset_map_.size());

    for( auto it = id_asset_map_.begin(), eit = id_asset_map_.end(); it != eit; ++it )
    {
        list.emplace_back(it->first);
    }

    return list;
}

QVector<QUuid> AssetCollection::idListForPrefix(const QString &fullPath) const
{
    QVector<QUuid> list;

    // TODO: this is currently a hack: reconstruct prefix from full path. unify this!
    QString prefix;
    if( fullPath == baseDir_.path() + "/" )
    {
        prefix = "";
    }
    else
    {
        auto basePath = baseDir_.path() + "/";

    //    assert( basePath.size() < fullPath.size());
        if( basePath.size() >= fullPath.size() )
        {
            return list;
        }
        prefix = fullPath.right(fullPath.size() - basePath.size());
    }


    auto first = prefixToIdMap.lowerBound(prefix);
    auto last = prefixToIdMap.upperBound(prefix);

    while( first != last )
    {
        list.push_back(first.value());
        ++first;
    }

    return list;
}

int AssetCollection::numIdsForPrefix(const QString &prefix) const
{
    return prefixToIdMap.count(prefix);
}

std::vector<std::string> AssetCollection::nameList()
{
    std::vector<std::string> list;

    list.reserve(id_asset_map_.size());

    for( auto it = id_asset_map_.begin(), eit = id_asset_map_.end(); it != eit; ++it )
    {
        Entry *ent = (it->second).get();
        cacheIn(ent);
        capnp::FlatArrayMessageReader fr(kj::ArrayPtr<const capnp::word>((capnp::word const *)ent->mappedData, ent->file.size() / sizeof(capnp::word)));
        Asset::Reader assetReader = fr.getRoot<Asset>();

        list.emplace_back(assetReader.getHeader().getName().cStr());

        //list.emplace_back(it->first);
    }

    return list;

}

//kj::ArrayPtr<const capnp::word> AssetCollection::at(int pos)
//{
//    const Entry &ent = *(assets_.at(pos));
//    const QByteArray &ba = ent.data;
//    const size_t size = ba.size();

//    return kj::ArrayPtr<const capnp::word>(reinterpret_cast<const capnp::word*>(ba.data()), size / sizeof(capnp::word));



////    capnp::ReaderOptions readerOptions;
////    readerOptions.traversalLimitInWords = 1024 * 1024 * 1024;
////    auto data = ::kj::heapArray<::kj::byte>(ba.size());
////    ::kj::ArrayInputStream ais(data);
////    ::capnp::InputStreamMessageReader r(ais, readerOptions);

////    return r.getRoot<Asset>();
//}

void AssetCollection::fullRescan()
{
    QDirIterator it(baseDir_.absolutePath(), QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);

    while(it.hasNext())
    {

        const QString &filename = it.next();

        if( filename.endsWith("index") )
        {
            continue;
        }


        QFile file(filename);



        QFileInfo fileinfo = it.fileInfo();
        QString relativePath = baseDir_.relativeFilePath(filename);

        //std::cout << (i++) << " " << it.next().toStdString().c_str() << std::endl;


        file.open(QFile::ReadOnly);

        if( !file.isReadable() )
        {
            continue;
        }



        //file.rea
        const size_t size = file.size();
        const uchar *ptr = file.map(0, size);

        //assets_.push_back(QVector<uchar>(ptr, ptr+size));


//        assets_.push_back(std::make_unique<Entry>(file.fileName()));

        capnp::FlatArrayMessageReader fr( kj::ArrayPtr<const capnp::word>(reinterpret_cast<const capnp::word*>(ptr), size / sizeof(capnp::word)));

        Asset::Reader assetReader = fr.getRoot<Asset>();


        QUuid uuid(assetReader.getHeader().getUuid().cStr());

//        std::cout << "uuid: " << uuid.toString().toStdString() << "\n";

        id_asset_map_.emplace(uuid, std::make_unique<Entry>(uuid, file.fileName()));
        relnameToIdMap.emplace(relativePath, uuid);
    }

    QSet<QString> prefixSet;
    //QVector<QString> prefixSet;
    //QMultiMap<QString


    for( auto it = relnameToIdMap.begin(), eit = relnameToIdMap.end(); it != eit; ++it )
    {
        QString name = it->first;
        int index = name.lastIndexOf('/');

        if( index != -1 )
        {
            name.truncate(index);
        }
        else
        {
            name = "";
        }

        // todo: de-duplacate prefix strings
        prefixToIdMap.insert(name, it->second);
    }
}

void AssetCollection::cacheIn(AssetCollection::Entry *ent)
{
    if(ent->mappedData != nullptr)
    {
        // already mapped -> move back in lru
//        auto lruMapIt = lruMap_.find(ent);
//        if( lruMapIt == lruMap_.end())
//        {
//            throw std::runtime_error( "lru inconsistent: mapped but not in queue" );
//        }
        if( ent->lruQueueIt->second != ent )
        {
            throw std::runtime_error( "lru inconsistent: wrong ent in queue" );
        }

        std::cout << "= " << (QDateTime::currentMSecsSinceEpoch()-ent->lruQueueIt->first) << " " << ent << std::endl;
        lruQueue_.erase(ent->lruQueueIt);

        ent->lruQueueIt = lruQueue_.emplace(QDateTime::currentMSecsSinceEpoch(), ent);
    }
    else
    {
        // not mapped

        // first check if we have to drop old entry from the cache
        const size_t LruThreshold = 128;
        if( lruQueue_.size() > LruThreshold )
        {
            auto lruQueueIt = lruQueue_.begin();
            Entry *uncacheEnt = lruQueueIt->second;
            if( uncacheEnt->mappedData == nullptr )
            {
                throw std::runtime_error( "lru inconsistent: unmapped entry in queue" );
            }
            std::cout << "- " << (QDateTime::currentMSecsSinceEpoch()-lruQueueIt->first) << " " << ent << std::endl;

            uncacheEnt->unmap();
            lruQueue_.erase(lruQueueIt);
            ent->lruQueueIt = lruQueue_.end();

        }

        // second: map entry and put into lru queue
        ent->map();

        ent->lruQueueIt = lruQueue_.emplace(QDateTime::currentMSecsSinceEpoch(), ent);
        std::cout << "+ " << ent << std::endl;
    }
}



AssetCollection::Entry::Entry(const QUuid &uuidX, const QString &filenameX)
    : uuid(uuidX)
    , filename(filenameX)
    , file(filename)
    , mappedData(nullptr)
{
  // data = file.readAll();

//    mappedData = file.map(0, file.size());
//    file.close();
}

void AssetCollection::Entry::map()
{
    if( mappedData == nullptr )
    {
        file.open(QFile::ReadOnly);

        mappedData = file.map(0, file.size());
        file.close();
    }
}

void AssetCollection::Entry::unmap()
{
    if( mappedData != nullptr)
    {
        file.unmap(mappedData);
        mappedData = nullptr;
    }
}
