#ifndef ELEMENTVIEWDELEGATE_H
#define ELEMENTVIEWDELEGATE_H

#include <QStyledItemDelegate>
#include "assetcollection.h"
#include "assetcollectionpreviewcache.h"

class ElementViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ElementViewDelegate( AssetCollectionPreviewCache & cache );
    ~ElementViewDelegate();

    enum datarole {headerTextRole = Qt::UserRole + 100,subHeaderTextrole = Qt::UserRole+101,IconRole = Qt::UserRole+102,RawDataRole = Qt::UserRole+103};

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    AssetCollectionPreviewCache & cache_;
//    AssetCollection const & collection_;
};

#endif // ELEMENTVIEWDELEGATE_H
