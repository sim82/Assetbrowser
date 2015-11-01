#include "AssetCollectionItemModel.h"
#include <iostream>

AssetCollectionItemModel::AssetCollectionItemModel()
    : QStandardItemModel()
{

}

AssetCollectionItemModel::~AssetCollectionItemModel()
{

}

bool AssetCollectionItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    std::cout << "dropMimeData" << std::endl;

    return true;
}

Qt::DropActions AssetCollectionItemModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

