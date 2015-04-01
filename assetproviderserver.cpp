#include "assetcollection.h"
#include "assetproviderserver.h"
#include "capnp/serialize.h"
#include "capnp/ez-rpc.h"
#include <QUuid>
#include <QPixmap>
#include <QRgb>
#include <iostream>

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

enum class pixel_format_type : uint32_t {
    RGBA,
    RGB,
    BGR,
    Luminance
};

static int pixelSize( pixel_format_type format )
{
    switch(format)
    {
    case pixel_format_type::RGBA:
        return 4;

    case pixel_format_type::RGB:
    case pixel_format_type::BGR:
        return 3;
    default:
    case pixel_format_type::Luminance:
        return 1;
    }
}

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


static pixel_format_type toPixelFormat( QImage::Format f )
{
    switch( f )
    {
    case QImage::Format_RGB888:
    case QImage::Format_RGB32:
        return pixel_format_type::RGB;

    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_RGBA8888:
        return pixel_format_type::RGBA;

    case QImage::Format_Indexed8:
        return pixel_format_type::Luminance;

    default:
        throw std::runtime_error( "format not implemented");
    }
}


class AssetProviderHandleImpl final: public AssetProvider::Handle::Server
{
public:
    AssetProviderHandleImpl(const AssetCollection::Entry &ent)
        : ent_(ent)
    {
    }

    ::kj::Promise<void> getAsset(GetAssetContext context) override
    {
        capnp::FlatArrayMessageReader fr(kj::ArrayPtr<const capnp::word>((capnp::word const *)ent_.mappedData, ent_.file.size() / sizeof(capnp::word)));
        Asset::Reader assetReader = fr.getRoot<Asset>();

        context.getResults().setAsset(assetReader);

        return kj::READY_NOW;
    }
    ::kj::Promise<void> getBaked(GetBakedContext context) override
    {
//        // just copy for now.
//        context.getResults().setBaked(kj::heap<AssetProviderHandleImpl>(ent_));

//        return kj::READY_NOW;
        try {
            return bakeImpl(ent_, context);
        } catch( std::runtime_error x )
        {
            std::cout << "bake error: " << x.what() << std::endl;

            capnp::FlatArrayMessageReader fr(kj::ArrayPtr<const capnp::word>((capnp::word const *)ent_.mappedData, ent_.file.size() / sizeof(capnp::word)));
            Asset::Reader assetReader = fr.getRoot<Asset>();

            context.getResults().setBaked(assetReader);
            return kj::READY_NOW;
        }
    }

private:

    kj::ArrayPtr<capnp::byte> toBakedPixelArray( QImage::Format inType, const QImage &image )
    {
        pixel_format_type outType = toPixelFormat(inType);

        auto ps = pixelSize(outType);

        std::vector<capnp::byte> tmp(ps * image.width() * image.height());
        //kj::ArrayPtr<capnp::byte> outBuf = kj::heapArray( tmp.data(), tmp.size() );
       // tmp.clear();

        auto outIt = tmp.begin(); //outBuf.begin();

        if( inType == QImage::Format_RGB32 || inType == QImage::Format_ARGB32 || inType == QImage::Format_ARGB32_Premultiplied )
        {
            for( int h = 0; h < image.height(); ++h )
            {
                QRgb *scanline = (QRgb*) image.scanLine(h);
                for( int w = 0; w < image.width(); ++w )
                {
                    *outIt++ = qRed(scanline[w]);
                    *outIt++ = qGreen(scanline[w]);
                    *outIt++ = qBlue(scanline[w]);

                    if( ps == 4 )
                    {
                        *outIt++ = qAlpha(scanline[w]);
                    }
                }
            }
        }
        else if( inType == QImage::Format_Indexed8 )
        {
            for( int h = 0; h < image.height(); ++h )
            {
                const uchar *scanline = image.scanLine(h);
                for( int w = 0; w < image.width(); ++w )
                {
                    *outIt++ = scanline[w];
                }
            }
        }
        else
        {
            throw std::runtime_error( "format not implemented");
        }
//        return std::move(outBuf);

        return kj::heapArray( tmp.data(), tmp.size() );
    }

    kj::Promise<void>bakeImpl(const AssetCollection::Entry &ent, GetBakedContext context )
    {
        capnp::FlatArrayMessageReader fr(kj::ArrayPtr<const capnp::word>((capnp::word const *)ent.mappedData, ent.file.size() / sizeof(capnp::word)));
        Asset::Reader assetReader = fr.getRoot<Asset>();

        if( !assetReader.hasPixelData() )
        {
            return kj::NEVER_DONE;
        }
        if( assetReader.getPixelData().hasCooked() )
        {
            // if it is already baked, just copy

            context.getResults().setBaked(assetReader);
            return kj::READY_NOW;
        }

        if( !assetReader.getPixelData().hasStored() )
        {
            return kj::NEVER_DONE;
        }
        AssetPixelDataStored::Reader storedReader = assetReader.getPixelData().getStored();

        const uchar *data = storedReader.getData().begin();
        const uint len = storedReader.getData().size();

        capnp::MallocMessageBuilder builder;
        Asset::Builder assetBuilder = builder.initRoot<Asset>();
        assetBuilder.setGuid(assetReader.getGuid());
        assetBuilder.setName(assetReader.getName());

        const char *dbgName = assetReader.getName().cStr();

        AssetPixelDataCooked::Builder cookedBuilder = assetBuilder.initPixelData().initCooked();

        QPixmap pixmap;
        pixmap.loadFromData(data, len, mimetypeToQtImageType(storedReader.getMimeType().begin()));

        QSize size = pixmap.size();
        int numMipmaps = 1;

        while( size.width() > 4 && size.height() > 4 )
        {
            ++numMipmaps;
            size /= 2;
        }

        cookedBuilder.initLevels(numMipmaps);
        cookedBuilder.initLevelData(numMipmaps);

        for( int i = 0; i < numMipmaps; ++i )
        {
            QSize size = pixmap.size();

            cookedBuilder.getLevels()[i].setWidth(size.width());
            cookedBuilder.getLevels()[i].setHeight(size.height());

            QImage image(pixmap.toImage());
            pixel_format_type pixelFormat = toPixelFormat(image.format());
            if( i == 0 )
            {
                cookedBuilder.setPixelFormat( uint32_t(pixelFormat));
            }

            cookedBuilder.getLevelData().set(i, toBakedPixelArray(image.format(), image));
            size /= 2;
            image = image.scaled(size);
        }

        return kj::READY_NOW;
    }

    const AssetCollection::Entry &ent_;
};

class AssetProviderImpl final: public AssetProvider::Server
{
public:
    AssetProviderImpl(AssetCollection &collection)
        : collection_(collection)
    {

    }

    ::kj::Promise<void> mapNameToUuid(MapNameToUuidContext context) override
    {
        const char *str = context.getParams().getName().cStr();
        setupNameToUuidMap();

        auto it = nameUuidMap_.find(str);
        if( it == nameUuidMap_.end() )
        {
            context.getResults().setUuid("<unknown>");
        }
        else
        {
            context.getResults().setUuid(it->second);
        }
        return kj::READY_NOW;
        //collection_.
    }

    ::kj::Promise<void> get(GetContext context) override
    {
        auto id = context.getParams().getUuid();
        auto &ent = collection_.entry(QUuid(id.cStr()));
        context.getResults().setHandle(kj::heap<AssetProviderHandleImpl>(ent));

        return kj::READY_NOW;
    }

    ::kj::Promise<void> getByName(GetByNameContext context) override
    {
        auto name = context.getParams().getName().cStr();

        setupNameToUuidMap();
        auto it = nameUuidMap_.find(name);

        if( it == nameUuidMap_.end())
        {
            return kj::NEVER_DONE;
        }

        auto &ent = collection_.entry(QUuid(it->second.c_str()));
        context.getResults().setHandle(kj::heap<AssetProviderHandleImpl>(ent));

        return kj::READY_NOW;
    }

    ::kj::Promise<void> nameList(NameListContext context) override
    {
        setupNameToUuidMap();
        capnp::List<capnp::Text>::Builder builder = context.getResults().initList(nameUuidMap_.size());

        size_t i = 0;
        for( auto it = nameUuidMap_.begin(), eit = nameUuidMap_.end(); it != eit; ++it )
        {
            builder.set(i, it->first.c_str());
            ++i;
        }


        return kj::READY_NOW;
    }

private:
    void setupNameToUuidMap()
    {
        if( !nameUuidMap_.empty())
        {
            return;
        }

        std::vector<QUuid> uuids = collection_.idList();
        std::vector<std::string> names = collection_.nameList();

        for( size_t i = 0; i < uuids.size(); ++i )
        {
            QUuid &id = uuids[i];
            QByteArray ba = id.toByteArray();
            nameUuidMap_.emplace(std::move(names.at(i)), std::string(ba.begin(), ba.end()));
        }
    }

    AssetCollection &collection_;

    std::map<std::string, std::string> nameUuidMap_;
};


AssetProviderServer::AssetProviderServer(AssetCollection &collection, QObject *parent)
    : QThread(parent)
    , collection_(collection)
{

}

AssetProviderServer::~AssetProviderServer()
{

}

void AssetProviderServer::run()
{
    capnp::EzRpcServer server("*", 12345);

    auto &waitScope = server.getWaitScope();

    server.exportCap("AssetProvider", kj::heap<AssetProviderImpl>(collection_));

    kj::NEVER_DONE.wait(waitScope);
}

