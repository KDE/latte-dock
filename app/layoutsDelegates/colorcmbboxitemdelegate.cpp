#include "colorcmbboxitemdelegate.h"

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QString>


ColorCmbBoxItemDelegate::ColorCmbBoxItemDelegate(QObject *parent, QString iconsPath)
    : QAbstractItemDelegate(parent),
      m_iconsPath(iconsPath)
{
}

QSize ColorCmbBoxItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.rect.width(), 50);
}

void ColorCmbBoxItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    QVariant value = index.data(Qt::DisplayRole);

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);

    if (value.isValid()) {
        QString colorPath = m_iconsPath + value.toString() + "print.jpg";
        QBrush colorBrush;
        colorBrush.setTextureImage(QImage(colorPath).scaled(QSize(50, 50)));

        painter->setBrush(colorBrush);
        painter->drawRect(option.rect - QMargins(5, 5, 5, 5));
    }


}

