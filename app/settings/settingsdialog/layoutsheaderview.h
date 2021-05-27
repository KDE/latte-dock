/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYOUTSHEADERVIEW_H
#define LAYOUTSHEADERVIEW_H

// Qt
#include <QHeaderView>

namespace Latte {
namespace Settings {
namespace Layouts {

class HeaderView : public QHeaderView
{
public:
    HeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
};

}
}
}
#endif
