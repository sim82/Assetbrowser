#include <QIcon>
#include <QApplication>
#include <QPainter>
#include "elementviewdelegate.h"
ElementViewDelegate::ElementViewDelegate()
{

}

ElementViewDelegate::~ElementViewDelegate()
{

}

QSize ElementViewDelegate::sizeHint(const QStyleOptionViewItem &  option ,
                                  const QModelIndex & index) const
{
   // QIcon icon = qvariant_cast<QIcon>(index.data(IconRole));
   // QSize iconsize = icon.actualSize(option.decorationSize);

    if( option.decorationSize.width() < 96 )
    {
        return option.decorationSize;
    }
    QFont font = QApplication::font();
    QFontMetrics fm(font);

   // return(QSize(iconsize.width(),iconsize.height()+fm.height() +8 ));

//    return icon.actualSize(option.decorationSize);
    return option.decorationSize + QSize(0, fm.height() +8) + QSize(16,16);
//    return(QSize(128, 128 ));
}

static QSize fitSize( const QSize &src, const QSize &dest )
{
    if( src.width() >= src.height() )
    {
        return QSize(dest.width(), src.height() * (float(dest.width()) / src.width()));
    }
    else
    {
        return QSize(src.width() * (float(dest.height()) / src.height()), dest.height());
    }
}

void ElementViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter,option,index);

    painter->save();

    QImage image = qvariant_cast<QImage>(index.data(IconRole));

    auto pixmap = QPixmap::fromImage(image.mirrored());

    QRect iconRect = option.rect;
    iconRect.adjust(8,8, -8, -8);


    if( option.decorationSize.width() < 96 )
    {
        painter->drawPixmap(iconRect, pixmap);
        //    painter->setFont(font);
        //    painter->drawText(headerRect,headerText);


        //    painter->setFont(SubFont);
        //    painter->drawText(subheaderRect.left(),subheaderRect.top()+17,subText);

        painter->restore();
        return;

    }
    QFont font = QApplication::font();
    QFont SubFont = QApplication::font();
    //font.setPixelSize(font.weight()+);
    font.setBold(true);
    SubFont.setWeight(SubFont.weight()-2);
    QFontMetrics fm(font);

    //QIcon icon = qvariant_cast<QIcon>(index.data(IconRole));
    //QPixmap pixmap = qvariant_cast<QPixmap>(index.data(IconRole));
    QString headerText = qvariant_cast<QString>(index.data(headerTextRole));
    QString subText = qvariant_cast<QString>(index.data(subHeaderTextrole));

    //QSize iconsize = icon.actualSize(option.decorationSize);

    //    QRect headerRect = option.rect;
    //    QRect subheaderRect = option.rect;

    //    iconRect.setRight(iconsize.width()+30);
    //    iconRect.setTop(iconRect.top()+5);
    //    headerRect.setLeft(iconRect.right());
    //    subheaderRect.setLeft(iconRect.right());
    //    headerRect.setTop(headerRect.top()+5);
    //    headerRect.setBottom(headerRect.top()+fm.height());

    //    subheaderRect.setTop(headerRect.bottom()+2);


    //painter->drawPixmap(QPoint(iconRect.right()/2,iconRect.top()/2),icon.pixmap(iconsize.width(),iconsize.height()));
    //painter->drawPixmap(QPoint(iconRect.left()+iconsize.width()/2+2,iconRect.top()+iconsize.height()/2+3),icon.pixmap(iconsize.width(),iconsize.height()));
    //painter->drawRect(option.rect);
  //  painter->fillRoundRect(option.rect);
    //painter->drawImage(QPoint(iconRect.left(),iconRect.top()),image);

    QSize size = fitSize( pixmap.size(), iconRect.size() );
    //    painter->drawPixmap(QRect(iconRect.left()
    //                              , iconRect.top()
    //                              , iconRect.width()
    //                              , iconRect.height())
    //                        ,pixmap);
    QRect headerRect(option.rect.left(), option.rect.top(), option.rect.width(), fm.height());
    headerRect.adjust(8, 0, -8, 0);
    painter->drawText(headerRect, Qt::AlignRight|Qt::AlignVCenter, headerText);
    painter->drawPixmap(QRect(iconRect.left()
                              , iconRect.top() + fm.height() + 8
                              , size.width()
                              , size.height())
                        ,pixmap);
    //    painter->setFont(font);
    //    painter->drawText(headerRect,headerText);


    //    painter->setFont(SubFont);
    //    painter->drawText(subheaderRect.left(),subheaderRect.top()+17,subText);

    painter->restore();

}

