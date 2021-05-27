/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYOUTCMBITEMDELEGATE_H
#define LAYOUTCMBITEMDELEGATE_H

// local
#include "../../../data/layouticondata.h"

// Qt
#include <QStyledItemDelegate>

class QModelIndex;
class QWidget;

namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

class LayoutCmbItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    LayoutCmbItemDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}
}
}
}

#endif
