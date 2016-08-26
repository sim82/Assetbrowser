#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "FlowLayout.h"
#include "CQt3dAssetViewer.h"


#include <QMainWindow>
#include <QMap>
#include <QSet>
#include <QUuid>
#include <QModelIndex>
#include <QPair>
#include <memory>
#include <vector>
class QStandardItemModel;

class BundleData;
class AssetProviderServer;
class AssetCollection;
class AssetCollectionPreviewCache;
class AssetCollectionOutlineModel;
class ElementViewDelegate;

class QFileSystemModel;
class QTimer;
class QStandardItem;

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
    void on_listView_doubleClicked(const QModelIndex &index);
    void on_previewCache_previewIconsChanged(QSet<QUuid> ids);
    void on_treeView_clicked(const QModelIndex &index);
    void on_treeView_activated(const QModelIndex &index);
    void on_treeView_pressed(const QModelIndex &index);
    void on_treeView_entered(const QModelIndex &index);
    void listviewScrollbar_valueChanged(int);
    void on_preloadTimer_timeout();
    void on_elementViewDelegate_itemPainted(QUuid id);

    void on_itemsTreeView_doubleClicked(const QModelIndex &index);

    void on_tabWidget_currentChanged(int index);

    void on_itemsTreeView_selection_currentChanged(const QModelIndex &index, const QModelIndex &prev);

    void on_listView_selection_currentChanged(const QModelIndex &index, const QModelIndex &prev);

    void on_pbViewAll_clicked();

private:
    void showQt3dAssetViewer();

    Ui::MainWindow *ui;
    ElementViewDelegate *elementViewDelegate;
    FlowLayout *flowLayout;
    QMap<QString, QStandardItemModel *> itemModels;

    QString currentItemModel;

    std::unique_ptr<BundleData> bundle;
    std::vector<AssetCollection *> collections_;
    AssetProviderServer *providerServer_;
    AssetCollectionPreviewCache *previewCache;
    //QMap<QUuid, int> idToRowMap;

    QMap<QUuid, QPair<QStandardItem *, AssetCollection*>> idToRowAndModelMap;

    QFileSystemModel *dirModel;

    AssetCollectionOutlineModel *outlineModel;

    QTimer *preloadTimer;
    QSet<QUuid> preloadSet;

    QModelIndex selectedIndex_;

    CQt3dAssetViewer *qt3dAssetViewer_{nullptr};
};

#endif // MAINWINDOW_H
