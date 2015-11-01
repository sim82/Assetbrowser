#include "AssetBrowserElement.h"
#include "ui_assetbrowserelement.h"

AssetBrowserElement::AssetBrowserElement(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AssetBrowserElement)
{
    ui->setupUi(this);
}

AssetBrowserElement::~AssetBrowserElement()
{
    delete ui;
}

void AssetBrowserElement::setTitle(const QString &title)
{
    ui->labelTitle->setText(title);
}

void AssetBrowserElement::setPixmap(const QPixmap &pm)
{
    ui->Preview->setPixmap(pm);
}

void AssetBrowserElement::setFromQImage(const QImage &image)
{
    ui->Preview->setPixmap(QPixmap::fromImage(image));
}
