/*
*  Copyright 2021 Michail Vourlakos <mvourlakos@gmail.com>
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
