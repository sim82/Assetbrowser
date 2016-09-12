#include "BrowserContent.h"
#include "ui_BrowserContent.h"

BrowserContent::BrowserContent(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BrowserContent)
{
    ui->setupUi(this);
}

BrowserContent::~BrowserContent()
{
    delete ui;
}
