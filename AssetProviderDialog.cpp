#include "AssetProviderDialog.h"
#include "ui_AssetProviderDialog.h"

AssetProviderDialog::AssetProviderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AssetProviderDialog)
{
    ui->setupUi(this);
}

AssetProviderDialog::~AssetProviderDialog()
{
    delete ui;
}
