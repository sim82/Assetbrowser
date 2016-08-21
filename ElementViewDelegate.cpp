#include <QIcon>
#include <QApplication>
#include <QPainter>
#include "ElementViewDelegate.h"
#include <capnp/serialize.h>

ElementViewDelegate::ElementViewDelegate()
{

}

ElementViewDelegate::~ElementViewDelegate()
{

}

static QRect getIconBox()
{
    QRect iconBox(0, 0, 64, 64);
    iconBox += QMargins(24, 8, 24, 8);

    return iconBox;
}


static QRect getTextBox()
{
    static QFont font = QApplication::font();
    static QFontMetrics fm(font);

    QRect textBox(0, 0, 64, fm.height() * 2);
    textBox += QMargins(24, 0, 24, 16);

    return textBox;
}

QSize ElementViewDelegate::sizeHint(const QStyleOptionViewItem &  option ,
                                  const QModelIndex & /*index*/) const
{
    auto iconBox = getIconBox();
    auto textBox = getTextBox();

    textBox.moveTopLeft(iconBox.bottomLeft());

    return ( iconBox | textBox ).size();
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

static const char *mimetypeToQtImageType(const char *mimetype)
{
    std::string s(mimetype);

    if( s == "image/png")
    {
        return "PNG";
    }
    else if( s == "image/jpeg" )
    {
        return "JPEG";
    }

    return nullptr;
}


void ElementViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    //QStyledItemDelegate::paint(painter,option,index);
    painter->save();
    {

        QUuid id = index.data(RawDataRole).toUuid();

#if 0
        cache_.request(id);
        QIcon & icon = cache_.get(id);

#else
        const QIcon& icon = qvariant_cast<QIcon>(index.data(IconRole));
#endif

        QString headerText = index.data(Qt::DisplayRole).toString();
        emit itemPainted(id);

//        cache_.use(id);


        QFont font = QApplication::font();
        QFont SubFont = QApplication::font();
        //font.setPixelSize(font.weight()+);
        font.setBold(true);
        SubFont.setWeight(SubFont.weight()-2);
        QFontMetrics fm(font);


        QRect iconBox(getIconBox());

        iconBox.moveTopLeft(option.rect.topLeft());

        QRect textBox(getTextBox());

        textBox.moveTopLeft(iconBox.bottomLeft());

        QSize iconTargetSize(64, 64);
        QSize iconSize = icon.actualSize(iconTargetSize);
        QRect iconRect = QRect(QPoint(0,0), iconSize);
        iconRect.moveCenter(iconBox.center());

        QRect textRect(QPoint(0,0), QSize(64 + 32, fm.height() * 2));
        textRect.moveCenter(textBox.center());
        textRect.moveTop( textBox.top() );

        painter->setPen(QPen(option.palette.windowText(), 1.0));
        painter->drawText(textRect, Qt::AlignHCenter|Qt::AlignTop|Qt::TextWrapAnywhere, headerText);

        //icon.actualSize()
        QImage image = icon.pixmap(iconRect.size()).toImage();


//        painter->setPen(QPen(QBrush(Qt::blue), 4.0));
        painter->setPen(QPen(QBrush(Qt::blue, Qt::Dense4Pattern), 4.0));

        QBrush bgBrush(Qt::Dense4Pattern);
        QMatrix bgMatrix;
        bgMatrix.scale(10, 10);
        bgBrush.setMatrix(bgMatrix);
        painter->setBrush(bgBrush);
        painter->drawRect(iconRect);


        painter->drawImage( iconRect
                            , image );
//        painter->setPen(QPen(QBrush(Qt::red), 1.0));
//        painter->drawRect(option.rect);

    }

    painter->restore();

}

