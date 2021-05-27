/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COLORCMBBOXITEMDELEGATE_H
#define COLORCMBBOXITEMDELEGATE_H

#include "../patternwidget.h"

// Qt
#include <QAbstractItemDelegate>

namespace Latte {
namespace Settings {
namespace Details {
namespace Delegate {

class ColorCmbBoxItem : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    ColorCmbBoxItem(QObject *parent = 0);
    ~ColorCmbBoxItem();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    Widget::PatternWidget *m_pattern{nullptr};
};

}
}
}
}

#endif
