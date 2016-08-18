#ifndef ASSETPREVIEWPIXELDATA_H
#define ASSETPREVIEWPIXELDATA_H

#include <QWidget>
#include "asset.capnp.h"

class QGraphicsScene;
class QGraphicsPixmapItem;

namespace Ui {
class AssetPreviewPixelData;
}

class AssetPreviewPixelData : public QWidget
{
    Q_OBJECT

public:
    explicit AssetPreviewPixelData(QWidget *parent = 0);
    ~AssetPreviewPixelData();
    void initFromAsset(cp::asset::AssetPixelData::Reader reader);
    void initFromImage(const QImage &image);

private slots:
    void on_horizontalSlider_sliderMoved(int position);

private:
    Ui::AssetPreviewPixelData *ui;

    QGraphicsScene *scene;
    qreal xoffset;
    std::vector<QGraphicsPixmapItem *> pixmapItems;
private:
};

#endif // ASSETPREVIEWPIXELDATA_H
