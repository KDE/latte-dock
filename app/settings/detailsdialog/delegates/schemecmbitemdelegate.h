/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCHEMECMBITEMDELEGATE_H
#define SCHEMECMBITEMDELEGATE_H

// Qt
#include <QStyledItemDelegate>

class QModelIndex;
class QWidget;

namespace Latte {
namespace Settings {
namespace Details {
namespace Delegate {

class SchemeCmbItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    SchemeCmbItemDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}
}
}
}

#endif
