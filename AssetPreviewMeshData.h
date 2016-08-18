#ifndef ASSETPREVIEWMESHDATA_H
#define ASSETPREVIEWMESHDATA_H

#include "asset.capnp.h"

#include <QWidget>

namespace Ui {
class AssetPreviewMeshData;
}

class AssetPreviewMeshData : public QWidget
{
    Q_OBJECT

public:
    explicit AssetPreviewMeshData(QWidget *parent = 0);
    ~AssetPreviewMeshData();

    void initFromAsset( cp::asset::AssetMeshData::Reader asset );

private:
    Ui::AssetPreviewMeshData *ui;
};

#endif // ASSETPREVIEWMESHDATA_H
