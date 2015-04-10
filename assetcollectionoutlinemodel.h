#ifndef ASSETCOLLECTIONOUTLINEMODEL_H
#define ASSETCOLLECTIONOUTLINEMODEL_H
#include <QAbstractItemModel>
class AssetCollection;
class CollectionDir;

class AssetCollectionOutlineModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    AssetCollectionOutlineModel( QObject *parent = nullptr );
    ~AssetCollectionOutlineModel();

    void addCollection( AssetCollection *collection );

    QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;


    QVector<QString> prefixList();
private:
    QVector<AssetCollection *> collections;
    QVector<CollectionDir *> rootDirs;
    QVector<CollectionDir *> dirs;
};

#endif // ASSETCOLLECTIONOUTLINEMODEL_H
