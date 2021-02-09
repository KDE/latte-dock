/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
