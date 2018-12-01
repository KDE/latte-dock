/*
*  Copyright 2017-2018 Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    QVariant data = index.data(Qt::UserRole);

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);

    if (value.isValid()) {
        QString dataStr = data.toString();
        QString valueStr = value.toString();

        if ((dataStr == "select_image") || (dataStr == "text_color")) {
            myOption.text = valueStr;
            QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
        } else {
            QString colorPath = valueStr.startsWith("/") ? valueStr : m_iconsPath + valueStr + "print.jpg";
            QBrush colorBrush;
            colorBrush.setTextureImage(QImage(colorPath).scaled(QSize(50, 50)));

            painter->setBrush(colorBrush);
            painter->drawRect(option.rect - QMargins(5, 5, 5, 5));
        }
    }

    painter->restore();
}

