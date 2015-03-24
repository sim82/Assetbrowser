#include <QDirIterator>
#include "assetcollection.h"
#include <iostream>
#include "asset.capnp.h"
#include "capnp/serialize.h"
#include <QVector>

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
    return assets_.size();
}

AssetCollection::Entry const & AssetCollection::entryAt(int pos) const
{
    return *(assets_.at(pos));
}

kj::ArrayPtr<const capnp::word> AssetCollection::at(int pos)
{
    const Entry &ent = *(assets_.at(pos));
    const QByteArray &ba = ent.data;
    const size_t size = ba.size();

    return kj::ArrayPtr<const capnp::word>(reinterpret_cast<const capnp::word*>(ba.data()), size / sizeof(capnp::word));



//    capnp::ReaderOptions readerOptions;
//    readerOptions.traversalLimitInWords = 1024 * 1024 * 1024;
//    auto data = ::kj::heapArray<::kj::byte>(ba.size());
//    ::kj::ArrayInputStream ais(data);
//    ::capnp::InputStreamMessageReader r(ais, readerOptions);

//    return r.getRoot<Asset>();
}

void AssetCollection::fullRescan()
{
    QDirIterator it(baseDir_.absolutePath(), QStringList() << "*.asset", QDir::Files, QDirIterator::Subdirectories);

    size_t i = 0;
    while(it.hasNext())
    {

        const QString &filename = it.next();
        QFile file(filename);

        //std::cout << (i++) << " " << it.next().toStdString().c_str() << std::endl;


        file.open(QFile::ReadOnly);

        if( !file.isReadable() )
        {
            continue;
        }



        //file.rea
        //const size_t size = file.size();
        //const uchar *ptr = file.map(0, size);

        //assets_.push_back(QVector<uchar>(ptr, ptr+size));


        assets_.push_back(std::make_unique<Entry>(file.fileName()));

        //capnp::FlatArrayMessageReader fr( kj::ArrayPtr<const capnp::word>(reinterpret_cast<const capnp::word*>(ptr), size / sizeof(capnp::word)));

        //Asset::Reader = fr.getRoot<Asset>();
    }
}



AssetCollection::Entry::Entry(const QString &filenameX)
    : filename(filenameX)
    , file(filename)
{
    file.open(QFile::ReadOnly);
  // data = file.readAll();

    mappedData = file.map(0, file.size());

}
