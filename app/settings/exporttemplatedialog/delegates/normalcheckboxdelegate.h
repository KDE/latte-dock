/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NORMALCHECKBOXDELEGATE_H
#define NORMALCHECKBOXDELEGATE_H

// Qt
#include <QStyledItemDelegate>


namespace Latte {
namespace Settings {
namespace Applets {
namespace Delegate {

class NormalCheckBox : public QStyledItemDelegate
{
public:
    NormalCheckBox(QObject *parent = 0);

    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

}
}
}
}

#endif
