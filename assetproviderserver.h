#ifndef ASSETPROVIDERSERVER_H
#define ASSETPROVIDERSERVER_H
#include "assetcollection.h"
#include <QThread>

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
