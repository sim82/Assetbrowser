#ifndef ASSETPREVIEWDIALOG_H
#define ASSETPREVIEWDIALOG_H

#include <QDialog>
#include "asset.capnp.h"

class QGraphicsScene;
class QGraphicsPixmapItem;

namespace Ui {
class AssetPreviewDialog;
}

class AssetPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AssetPreviewDialog(QWidget *parent = 0);
    ~AssetPreviewDialog();

    void initFromAsset(cp::asset::Asset::Reader reader);

private:
    Ui::AssetPreviewDialog *ui;
    QGraphicsScene *scene;
    qreal xoffset;
    std::vector<QGraphicsPixmapItem *> pixmapItems;
    QWidget *widget_{nullptr};
};

#endif // ASSETPREVIEWDIALOG_H
