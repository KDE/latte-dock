/*
    SPDX-FileCopyrightText: 2017-2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CHECKBOXDELEGATE_H
#define CHECKBOXDELEGATE_H

// Qt
#include <QStyledItemDelegate>


namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

class CheckBox : public QStyledItemDelegate
{
public:
    CheckBox(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

}
}
}
}

#endif
