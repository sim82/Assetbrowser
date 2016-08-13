#ifndef ASSETCOLLECTIONPREVIEWCACHE_H
#define ASSETCOLLECTIONPREVIEWCACHE_H

#include "AssetCollection.h"

#include <QObject>
#include <QUuid>
#include <map>
#include <set>
#include <QIcon>
#include <stack>
#include <QSet>

class QTimer;
class AssetCollectionPreviewCache : public QObject
{
    Q_OBJECT
public:
    explicit AssetCollectionPreviewCache(QObject *parent = 0);
    ~AssetCollectionPreviewCache();

    void addCollection(AssetCollection *ac);
    bool request( QUuid const& id );
    QIcon &get( QUuid const& id);
    void use( const QUuid &id );
signals:
    void previewIconsChanged(QSet<QUuid> ids);

public slots:
    void on_timer_timeout();

private:
    const AssetCollection::Entry &findEntry(const QUuid &id);

    QIcon previewIcon;
    QIcon noPreviewIcon;

    std::vector<AssetCollection*> collections_;
    std::map<QUuid, AssetCollection*> idToCollection_;
    std::map<QUuid, QIcon> cache_;

    std::stack<QUuid> lifoQueue_;

    QTimer *timer;
};

#endif // ASSETCOLLECTIONPREVIEWCACHE_H
