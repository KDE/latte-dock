/*
    SPDX-FileCopyrightText: 2017-2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYOUTBACKGROUNDDELEGATE_H
#define LAYOUTBACKGROUNDDELEGATE_H

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

class BackgroundDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    BackgroundDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

};

}
}
}
}

#endif
