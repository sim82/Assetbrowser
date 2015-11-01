#ifndef ASSETCOLLECTION_H
#define ASSETCOLLECTION_H

#include <QObject>
#include <QString>
#include <QDir>
#include "asset.capnp.h"
#include "kj/array.h"
#include <memory>
#include <vector>
#include <map>
#include <QUuid>
#include <QMultiMap>
#include <unordered_map>

class AssetCollection : public QObject
{
    Q_OBJECT
public:
    struct Entry;
    typedef std::multimap<qint64,Entry *> LruQueueType;

    struct Entry {
        Entry( QUuid const & uuid, QString const & filename );

        QUuid uuid;
        QString filename;
        QByteArray data;
        QFile file;
        uchar * mappedData;

        LruQueueType::iterator lruQueueIt;

        void map();
        void unmap();
    };

    explicit AssetCollection(const char *path, QObject *parent = 0);
    ~AssetCollection();

    size_t size() const;
//    kj::ArrayPtr<const capnp::word> at(int pos);

//    const AssetCollection::Entry &entryAt(int pos) const;
    const AssetCollection::Entry &entry(QUuid const& id);

    std::vector<QUuid> idList() const;
    QVector<QUuid> idListForPrefix( const QString &prefix ) const;
    int numIdsForPrefix( const QString &prefix ) const;
    std::vector<std::string> nameList();

    QDir baseDir() {
        return baseDir_;
    }

signals:

public slots:
    void fullRescan();

private:
    void cacheIn(Entry *ent);

//    QString path_;
    QDir baseDir_;



//    std::vector<std::unique_ptr<Entry>> assets_;

    std::map<QUuid, std::unique_ptr<Entry>> id_asset_map_;
    //typedef std::map<QUuid, std::unique_ptr<Entry>> IdAssetMapType;

    std::map<QString, QUuid> relnameToIdMap;
    QMultiMap<QString, QUuid> prefixToIdMap;

    LruQueueType lruQueue_;

//    std::unordered_map<Entry*, LruQueueType::iterator> lruMap_;

};

#endif // ASSETCOLLECTION_H
