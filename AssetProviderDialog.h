#ifndef ASSETPROVIDERDIALOG_H
#define ASSETPROVIDERDIALOG_H

#include <QDialog>

namespace Ui {
class AssetProviderDialog;
}

class AssetProviderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AssetProviderDialog(QWidget *parent = 0);
    ~AssetProviderDialog();

private:
    Ui::AssetProviderDialog *ui;
};

#endif // ASSETPROVIDERDIALOG_H
