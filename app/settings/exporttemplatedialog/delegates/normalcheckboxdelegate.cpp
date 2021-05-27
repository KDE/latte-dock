/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
