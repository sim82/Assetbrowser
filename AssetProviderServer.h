#ifndef ASSETPROVIDERSERVER_H
#define ASSETPROVIDERSERVER_H
#include "AssetCollection.h"
#include <QThread>
#include <asset.capnp.h>


void bakeImpl(cp::asset::Asset::Reader assetReader, cp::asset::Asset::Builder assetBuilder , bool smooth = true);

class AssetProviderServer
        : public QThread
{
    Q_OBJECT
public:
    AssetProviderServer( AssetCollection &collection, QObject *parent = nullptr);
    ~AssetProviderServer();

private:
    void run();
    AssetCollection &collection_;
};

#endif // ASSETPROVIDERSERVER_H
