#include <QIcon>
#include <QApplication>
#include <QPainter>
#include "elementviewdelegate.h"
#include <capnp/serialize.h>

ElementViewDelegate::ElementViewDelegate( AssetCollectionPreviewCache &cache)
    : cache_(cache)
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
    QStyledItemDelegate::paint(painter,option,index);


    painter->save();




//    if( !index.data(IconRole).isNull())
//    {
//        QImage image = qvariant_cast<QImage>(index.data(IconRole));

//        auto pixmap = QPixmap::fromImage(image.mirrored());

//        QRect iconRect = option.rect;
//        iconRect.adjust(8,8, -8, -8);


//        if( option.decorationSize.width() < 96 )
//        {
//            painter->drawPixmap(iconRect, pixmap);
//            //    painter->setFont(font);
//            //    painter->drawText(headerRect,headerText);


//            //    painter->setFont(SubFont);
//            //    painter->drawText(subheaderRect.left(),subheaderRect.top()+17,subText);

//            painter->restore();
//            return;

//        }
//        QFont font = QApplication::font();
//        QFont SubFont = QApplication::font();
//        //font.setPixelSize(font.weight()+);
//        font.setBold(true);
//        SubFont.setWeight(SubFont.weight()-2);
//        QFontMetrics fm(font);

//        //QIcon icon = qvariant_cast<QIcon>(index.data(IconRole));
//        //QPixmap pixmap = qvariant_cast<QPixmap>(index.data(IconRole));
//        QString headerText = qvariant_cast<QString>(index.data(headerTextRole));
//        QString subText = qvariant_cast<QString>(index.data(subHeaderTextrole));



//        QSize size = fitSize( pixmap.size(), iconRect.size() );
//        //    painter->drawPixmap(QRect(iconRect.left()
//        //                              , iconRect.top()
//        //                              , iconRect.width()
//        //                              , iconRect.height())
//        //                        ,pixmap);
//        QRect headerRect(option.rect.left(), option.rect.top(), option.rect.width(), fm.height());
//        headerRect.adjust(8, 0, -8, 0);
//        painter->drawText(headerRect, Qt::AlignRight|Qt::AlignVCenter, headerText);
//        painter->drawPixmap(QRect(iconRect.left()
//                                  , iconRect.top() + fm.height() + 8
//                                  , size.width()
//                                  , size.height())
//                        ,pixmap);
//    }
//    else if( !index.data(RawDataRole).isNull())
    {
        QUuid id = index.data(RawDataRole).toUuid();

//        auto const & ent = collection_.entry(id);

//        capnp::FlatArrayMessageReader fr(kj::ArrayPtr<const capnp::word>((capnp::word const *)ent.mappedData, ent.file.size() / sizeof(capnp::word)));
//        Asset::Reader assetReader = fr.getRoot<Asset>();


//        if( !assetReader.hasPixelData() )
//        {
//            return;
//        }
//        if( !assetReader.getPixelData().hasStored() )
//        {
//            return;
//        }




//        AssetPixelDataStored::Reader storedReader = assetReader.getPixelData().getStored();
//        const uchar *data = storedReader.getData().begin();
//        const uint len = storedReader.getData().size();

//        QPixmap pixmap;
//        pixmap.loadFromData(data, len, mimetypeToQtImageType(storedReader.getMimeType().begin()));



//        QString headerText = assetReader.getName().cStr();

//        QRect iconRect = option.rect;
//        iconRect.adjust(8,8, -8, -8);

//        QSize size = fitSize( pixmap.size(), iconRect.size() );
//                //    painter->drawPixmap(QRect(iconRect.left()
//                //                              , iconRect.top()
//                //                              , iconRect.width()
//                //                              , iconRect.height())
//                //                        ,pixmap);


        QFont font = QApplication::font();
        QFont SubFont = QApplication::font();
        //font.setPixelSize(font.weight()+);
        font.setBold(true);
        SubFont.setWeight(SubFont.weight()-2);
        QFontMetrics fm(font);

        cache_.request(id);
        QIcon & icon = cache_.get(id);

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

//        painter->setPen(Qt::NoPen);
//        painter->setBrush(option.palette.window());
//        painter->drawRect(option.rect);

//        QRect bgRect = option.rect;
//        bgRect -= QMargins(1, 1, 1, 1);
//        bgRect.moveCenter(option.rect.center());
//        painter->setBrush(QBrush(Qt::gray));
//        painter->drawRect(bgRect);

        QString headerText = id.toString();
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

