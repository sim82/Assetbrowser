#ifndef ASSETPROVIDERSERVER_H
#define ASSETPROVIDERSERVER_H
#include "assetcollection.h"
#include <QThread>
#include <asset.capnp.h>

void bakeImpl(Asset::Reader assetReader, Asset::Builder assetBuilder , bool smooth = true);

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
