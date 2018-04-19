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

#include "checkboxdelegate.h"

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QStandardItemModel>

const int HIDDENTEXTCOLUMN = 1;

CheckBoxDelegate::CheckBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStandardItemModel *model = (QStandardItemModel *) index.model();
    QStyledItemDelegate::paint(painter, option, model->index(index.row(), HIDDENTEXTCOLUMN));

    QStyledItemDelegate::paint(painter, option, index);
}

bool CheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                   const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

//     // make sure that the item is checkable
//     Qt::ItemFlags flags = model->flags(index);
//
//     if (!(flags & Qt::ItemIsUserCheckable) || !(flags & Qt::ItemIsEnabled))
//         return false;
//
//     // make sure that we have a check state
    QString value{index.data().toString()};
//
//     if (!value.isValid())
//         return false;

    // make sure that we have the right event type
    if (event->type() == QEvent::MouseButtonDblClick) {
        if (!option.rect.contains(static_cast<QMouseEvent *>(event)->pos()))
            return false;
    } else if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent *>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent *>(event)->key() != Qt::Key_Select)
            return false;
    } else {
        return false;
    }

    const QChar CheckMark{0x2714};
    return model->setData(index, value == CheckMark ? QString("") : CheckMark, Qt::DisplayRole);
}
