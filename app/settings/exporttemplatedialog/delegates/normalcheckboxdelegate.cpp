/*
*  Copyright 2021 Michail Vourlakos <mvourlakos@gmail.com>
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

#include "normalcheckboxdelegate.h"

// Qt
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>

namespace Latte {
namespace Settings {
namespace Applets {
namespace Delegate {

NormalCheckBox::NormalCheckBox(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

bool NormalCheckBox::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                 const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

    if (event->type() == QEvent::MouseButtonDblClick) {
        if (!option.rect.contains(static_cast<QMouseEvent *>(event)->pos())){
            return false;
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QRect checkBoxRect{option.rect.x(), option.rect.y(), option.rect.x() + option.rect.height(), option.rect.y() + option.rect.height()};
        if (!checkBoxRect.contains(static_cast<QMouseEvent *>(event)->pos())){
            return false;
        }
    } else if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent *>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent *>(event)->key() != Qt::Key_Select)
            return false;
    } else {
        return false;
    }

    const int currentState = index.data(Qt::CheckStateRole).toInt();
    return model->setData(index, (currentState>1 ? Qt::Unchecked : Qt::Checked), Qt::CheckStateRole);
}

}
}
}
}
