#ifndef ASSETCOLLECTIONPREVIEWCACHE_H
#define ASSETCOLLECTIONPREVIEWCACHE_H

#include <QObject>
#include <QUuid>
#include <map>
#include <QIcon>

class AssetCollection;
class AssetCollectionPreviewCache : public QObject
{
    Q_OBJECT
public:
    explicit AssetCollectionPreviewCache(AssetCollection & collection, QObject *parent = 0);
    ~AssetCollectionPreviewCache();

    bool request( QUuid const& id );
    QIcon &get( QUuid const& id);
signals:

public slots:

private:
    AssetCollection & collection_;
    std::map<QUuid, QIcon> cache_;
};

#endif // ASSETCOLLECTIONPREVIEWCACHE_H
