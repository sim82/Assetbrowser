#ifndef ASSETBROWSERELEMENT_H
#define ASSETBROWSERELEMENT_H

#include <QWidget>

namespace Ui {
class AssetBrowserElement;
}

class AssetBrowserElement : public QWidget
{
    Q_OBJECT

public:
    explicit AssetBrowserElement(QWidget *parent = 0);
    ~AssetBrowserElement();

    void setTitle( const QString &title );
    void setPixmap( const QPixmap &pm );
    void setFromQImage( const QImage &image );
private:
    Ui::AssetBrowserElement *ui;
};

#endif // ASSETBROWSERELEMENT_H
