#include "layoutnamedelegate.h"

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

LayoutNameDelegate::LayoutNameDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void LayoutNameDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;

    bool isLocked = index.data(Qt::UserRole).toBool();

    if (isLocked) {
        int thick = option.rect.height();
        myOption.rect.setWidth(option.rect.width() - thick);

        QStyledItemDelegate::paint(painter, myOption, index);

        QIcon lockIcon = QIcon::fromTheme("object-locked");
        QRect destination(option.rect.x() + option.rect.width() - thick, option.rect.y(), thick, thick);
        painter->drawPixmap(destination, lockIcon.pixmap(thick, thick));
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

