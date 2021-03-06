#include "AssetCollection.h"
#include "AssetProviderServer.h"
#include "capnp/serialize.h"
#include "capnp/ez-rpc.h"
#include <QUuid>
#include <QPixmap>
#include <QRgb>
#include <iostream>

using namespace cp::asset;

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
    //case QImage::Format_RGB32:
        return pixel_format_type::RGB;

//    case QImage::Format_ARGB32:
//    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_RGBA8888:
        return pixel_format_type::RGBA;

    case QImage::Format_Indexed8:
        return pixel_format_type::Luminance;

    default:
        throw std::runtime_error( "format not implemented");
    }
}


static kj::Array<capnp::byte> toBakedPixelArray( QImage::Format inType, const QImage &image )
{
    pixel_format_type outType = toPixelFormat(inType);

    auto ps = pixelSize(outType);

    std::vector<capnp::byte> tmp(ps * image.width() * image.height());
    //kj::ArrayPtr<capnp::byte> outBuf = kj::heapArray( tmp.data(), tmp.size() );
   // tmp.clear();

    auto outIt = tmp.begin(); //outBuf.begin();

    int bps = image.bytesPerLine();

    if( ps * image.width() > bps )
    {
        throw std::runtime_error( "internal error: inconsistent QImage pixel size");
    }

    for( int h = 0; h < image.height(); ++h )
    {
        const uchar *scanline = image.constScanLine(h);
        auto lineSize = ps * image.width();
        outIt = std::copy(scanline, scanline + lineSize, outIt);
    }
//        return std::move(outBuf);

    return kj::heapArray( tmp.data(), tmp.size() );
}

void bakeImpl(Asset::Reader assetReader, Asset::Builder assetBuilder, bool smooth )
{
    AssetPixelDataStored::Reader storedReader = assetReader.getPixelData().getStored();

    if( !storedReader.hasData())
    {
        throw std::runtime_error("cannot get data form stored asset");
    }

    const uchar *data = storedReader.getData().begin();
    const uint len = storedReader.getData().size();

    assetBuilder.setHeader(assetReader.getHeader());
    const char *dbgName = assetReader.getHeader().getName().cStr();

    AssetPixelDataCooked::Builder cookedBuilder = assetBuilder.initPixelData().initCooked();

//        QPixmap pixmap;
//        pixmap.loadFromData(data, len, mimetypeToQtImageType(storedReader.getMimeType().begin()));

    QImage image;
    image.loadFromData(data, len/*, mimetypeToQtImageType(storedReader.getMimeType().begin())*/);
    auto inType = image.format();
    if( inType == QImage::Format_Invalid )
    {
        throw std::runtime_error("invalid image format");
    }

    if( inType == QImage::Format_RGB32 )
    {
        image = image.convertToFormat(QImage::Format_RGB888);
    }
    else if( inType == QImage::Format_ARGB32 || inType == QImage::Format_ARGB32_Premultiplied)
    {
        image = image.convertToFormat(QImage::Format_RGBA8888);
    }

    QSize size = image.size();
    int numMipmaps = 1;

    while( size.width() > 4 && size.height() > 4 )
    {
        ++numMipmaps;
        size /= 2;
    }

    cookedBuilder.initLevels(numMipmaps);
    cookedBuilder.initLevelData(numMipmaps);

    size = image.size();
    for( int i = 0; i < numMipmaps; ++i )
    {
        cookedBuilder.getLevels()[i].setWidth(size.width());
        cookedBuilder.getLevels()[i].setHeight(size.height());

        pixel_format_type pixelFormat = toPixelFormat(image.format());
        if( i == 0 )
        {
            cookedBuilder.setPixelFormat( uint32_t(pixelFormat));
        }

        QImage outImage;
        if( i == 0 )
        {
            outImage = image;
        }
        else {
            auto quality = (smooth && image.format() !=QImage::Format_Indexed8) ? Qt::SmoothTransformation : Qt::FastTransformation;
            outImage = image.scaled(size, Qt::IgnoreAspectRatio, quality);
            auto format = outImage.format();
            if( format != image.format()) {
//                QImage xxx = outImage.convertToFormat(image.format());
//                if( xxx.format() != image.format() )
//                {
//                    throw std::runtime_error("convertToFormat failed");
//                }

                outImage = outImage.convertToFormat(image.format());


            }

//            if( format == QImage::Format_RGB32 )
//            {

//            }
//            else if( format == QImage::Format_ARGB32 || format == QImage::Format_ARGB32_Premultiplied)
//            {
//                outImage = outImage.convertToFormat(QImage::Format_RGBA8888);
//            }
        }
        QImage::Format outFormat = outImage.format();

        if( outFormat != image.format() )
        {
            throw std::runtime_error("convertToFormat failed");
        }

        auto array = toBakedPixelArray(outFormat, outImage);
        cookedBuilder.getLevelData().set(i, std::move(array));
        size /= 2;
    }
}


class AssetProviderInvalidHandleImpl final: public AssetProvider::Handle::Server
{
public:
    ::kj::Promise<void> isValid(IsValidContext context) override
    {
        context.getResults().setValid(false);
        return kj::READY_NOW;
    }
};

class AssetProviderHandleImpl final: public AssetProvider::Handle::Server
{
public:
    AssetProviderHandleImpl(const AssetCollection::Entry &ent)
        : ent_(ent)
    {
    }

    ::kj::Promise<void> isValid(IsValidContext context) override
    {
        context.getResults().setValid(true);
        return kj::READY_NOW;
    }

    ::kj::Promise<void> getId(GetIdContext context) override
    {
        context.getResults().setUuid(ent_.uuid.toString().toUtf8().constData());
        return kj::READY_NOW;
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
            capnp::FlatArrayMessageReader fr(kj::ArrayPtr<const capnp::word>((capnp::word const *)ent_.mappedData, ent_.file.size() / sizeof(capnp::word)));
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
            capnp::MallocMessageBuilder builder;
            Asset::Builder assetBuilder = builder.initRoot<Asset>();

            bakeImpl(assetReader, assetBuilder);
            context.getResults().setBaked(assetBuilder);

            return kj::READY_NOW;
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
        try
        {
            auto &ent = collection_.entry(QUuid(id.cStr()));
            context.getResults().setHandle(kj::heap<AssetProviderHandleImpl>(ent));
        }
        catch( std::runtime_error x )
        {
            std::cout << "exception: " << x.what() << std::endl;
            context.getResults().setHandle(kj::heap<AssetProviderInvalidHandleImpl>());
        }

        return kj::READY_NOW;
    }

    ::kj::Promise<void> getByName(GetByNameContext context) override
    {
        auto name = context.getParams().getName().cStr();

        setupNameToUuidMap();
        auto it = nameUuidMap_.find(name);

        if( it == nameUuidMap_.end())
        {
            context.getResults().setHandle(kj::heap<AssetProviderInvalidHandleImpl>());
            return kj::READY_NOW;
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
    capnp::EzRpcServer server("127.0.0.1", 12345);

    auto &waitScope = server.getWaitScope();

    server.exportCap("AssetProvider", kj::heap<AssetProviderImpl>(collection_));

    kj::NEVER_DONE.wait(waitScope);
}

