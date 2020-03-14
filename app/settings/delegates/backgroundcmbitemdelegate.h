/*
*  Copyright 2017-2018 Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef BACKGROUNDCMBBOXITEMDELEGATE_H
#define BACKGROUNDCMBBOXITEMDELEGATE_H

// Qt
#include <QAbstractItemDelegate>

namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

class BackgroundCmbBoxItem : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    BackgroundCmbBoxItem(QObject *parent = 0, QString iconsPath = QString());

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QString m_iconsPath;

};

}
}
}
}

#endif
