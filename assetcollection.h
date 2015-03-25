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
class AssetCollection : public QObject
{
    Q_OBJECT
public:
    struct Entry {
        Entry( QString const & filename );

        QString filename;
        QByteArray data;
        QFile file;
        uchar const * mappedData;

    };

    explicit AssetCollection(const char *path, QObject *parent = 0);
    ~AssetCollection();

    size_t size() const;
//    kj::ArrayPtr<const capnp::word> at(int pos);

//    const AssetCollection::Entry &entryAt(int pos) const;
    const AssetCollection::Entry &entry(QUuid const& id) const;

    std::vector<QUuid> idList() const;
signals:

public slots:
    void fullRescan();

private:
//    QString path_;
    QDir baseDir_;



//    std::vector<std::unique_ptr<Entry>> assets_;

    std::map<QUuid, std::unique_ptr<Entry>> id_asset_map_;
};

#endif // ASSETCOLLECTION_H
