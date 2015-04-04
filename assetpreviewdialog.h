#ifndef ASSETPREVIEWDIALOG_H
#define ASSETPREVIEWDIALOG_H

#include <QDialog>
#include "asset.capnp.h"

class QGraphicsScene;

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
private slots:
    void on_horizontalSlider_sliderMoved(int position);

private:
    Ui::AssetPreviewDialog *ui;
    QGraphicsScene *scene;
    qreal xoffset;
};

#endif // ASSETPREVIEWDIALOG_H
