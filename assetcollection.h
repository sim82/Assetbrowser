#ifndef ASSETCOLLECTION_H
#define ASSETCOLLECTION_H

#include <QObject>
#include <QString>
#include <QDir>
#include "asset.capnp.h"
#include "kj/array.h"
#include <memory>
#include <vector>

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
    kj::ArrayPtr<const capnp::word> at(int pos);

    const AssetCollection::Entry &entryAt(int pos) const;
signals:

public slots:
    void fullRescan();

private:
//    QString path_;
    QDir baseDir_;



    std::vector<std::unique_ptr<Entry>> assets_;
};

#endif // ASSETCOLLECTION_H
