#ifndef ASSETCOLLECTIONITEMMODEL_H
#define ASSETCOLLECTIONITEMMODEL_H

#include "QStandardItemModel"

class AssetCollectionItemModel
        : public QStandardItemModel
{
public:
    AssetCollectionItemModel();
    ~AssetCollectionItemModel();
    bool dropMimeData(const QMimeData *data,
         Qt::DropAction action, int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;
};

#endif // ASSETCOLLECTIONITEMMODEL_H
