#include "colorcmbboxitemdelegate.h"

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QString>


ColorCmbBoxItemDelegate::ColorCmbBoxItemDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

void ColorCmbBoxItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    //QString text = Items[index.row()].c_str();
    //myOption.text = text;
    QVariant value = index.data(Qt::DisplayRole);

    if (value.isValid()) {
        qDebug() << value.toString();
        /*QString colorPath = m_iconsPath + value.toString() + "print.jpg";
        QBrush colorBrush;
        colorBrush.setTextureImage(QImage(colorPath).scaled(QSize(50, 50)));

        painter->setBrush(colorBrush);
        painter->drawRect(QRect(option.rect.x(), option.rect.y(),
                                option.rect.width(), option.rect.height()));*/
    }

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
}

