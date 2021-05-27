/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SINGLETEXTDELEGATE_H
#define SINGLETEXTDELEGATE_H

// Qt
#include <QPainter>
#include <QStyledItemDelegate>

class QModelIndex;
class QWidget;
class QVariant;

namespace Latte {
namespace Settings {
namespace View {
namespace Delegate {

class SingleText : public QStyledItemDelegate
{
    Q_OBJECT

public:
    SingleText(QObject *parent);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}
}
}
}

#endif
