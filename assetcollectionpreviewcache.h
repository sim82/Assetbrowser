#ifndef ASSETCOLLECTIONPREVIEWCACHE_H
#define ASSETCOLLECTIONPREVIEWCACHE_H

#include <QObject>
#include <QUuid>
#include <map>
#include <QIcon>
#include <stack>
#include <QSet>

class QTimer;
class AssetCollection;
class AssetCollectionPreviewCache : public QObject
{
    Q_OBJECT
public:
    explicit AssetCollectionPreviewCache(AssetCollection & collection, QObject *parent = 0);
    ~AssetCollectionPreviewCache();

    bool request( QUuid const& id );
    QIcon &get( QUuid const& id);
    void use( const QUuid &id );
signals:
    void previewIconsChanged(QSet<QUuid> ids);

public slots:
    void on_timer_timeout();

private:
    QIcon previewIcon;
    QIcon noPreviewIcon;

    AssetCollection & collection_;
    std::map<QUuid, QIcon> cache_;

    std::stack<QUuid> lifoQueue_;

    QTimer *timer;
};

#endif // ASSETCOLLECTIONPREVIEWCACHE_H
