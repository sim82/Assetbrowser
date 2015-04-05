#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QUuid>
#include <memory>
#include "flowlayout.h"
class QStandardItemModel;

class BundleData;
class AssetProviderServer;
class AssetCollection;
class AssetCollectionPreviewCache;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_horizontalSlider_actionTriggered(int action);

    void on_listView_activated(const QModelIndex &index);

    void on_listView_doubleClicked(const QModelIndex &index);

    void on_previewCache_previewIconsChanged(QSet<QUuid> ids);
private:
    Ui::MainWindow *ui;
    FlowLayout *flowLayout;
    QStandardItemModel *itemModel;

    std::unique_ptr<BundleData> bundle;
    AssetCollection * ac;
    AssetProviderServer *providerServer_;
    AssetCollectionPreviewCache *previewCache;
    QMap<QUuid, int> idToRowMap;
};

#endif // MAINWINDOW_H
