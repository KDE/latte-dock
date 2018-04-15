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

