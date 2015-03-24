#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "flowlayout.h"
class QStandardItemModel;

class BundleData;

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

private:
    Ui::MainWindow *ui;
    FlowLayout *flowLayout;
    QStandardItemModel *itemModel;

    std::unique_ptr<BundleData> bundle;
};

#endif // MAINWINDOW_H
