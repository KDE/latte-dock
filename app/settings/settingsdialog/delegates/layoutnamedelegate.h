/*
    SPDX-FileCopyrightText: 2017-2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYOUTNAMEDELEGATE_H
#define LAYOUTNAMEDELEGATE_H

// Qt
#include <QStyledItemDelegate>

namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

class LayoutName : public QStyledItemDelegate
{
public:
    LayoutName(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}
}
}
}
#endif
