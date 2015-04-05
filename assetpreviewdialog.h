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

    void initFromAsset(Asset::Reader reader);
    void initFromImage(const QImage &image);
private slots:
    void on_horizontalSlider_sliderMoved(int position);

private:
    Ui::AssetPreviewDialog *ui;
    QGraphicsScene *scene;
    qreal xoffset;
    std::vector<QGraphicsPixmapItem *> pixmapItems;
};

#endif // ASSETPREVIEWDIALOG_H
