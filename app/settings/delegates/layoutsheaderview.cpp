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

#include "layoutsheaderview.h"

// local
#include "../models/layoutsmodel.h"

// Qt
#include <QAbstractItemModel>
#include <QPainter>


namespace Latte {
namespace Settings {
namespace Layouts {

HeaderView::HeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setSectionsClickable(true);
    setSectionsMovable(true);
    setSortIndicatorShown(true);
}

void HeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    if (logicalIndex == Model::Layouts::BACKGROUNDCOLUMN) {
        QString text = model()->headerData(Model::Layouts::BACKGROUNDCOLUMN, Qt::Horizontal, Qt::DisplayRole).toString();
        QIcon icon = model()->headerData(Model::Layouts::BACKGROUNDCOLUMN, Qt::Horizontal, Qt::DecorationRole).value<QIcon>();

        if (text.isEmpty() && !icon.isNull()) {
            //! draw centered icon
            QHeaderView::paintSection(painter, rect, Model::Layouts::HIDDENTEXTCOLUMN);

            int margin = 4;
            int thick = rect.height() - 2*margin;
            int iX = rect.x() + (rect.width()/2) - (thick/2);

            painter->drawPixmap(QRect(iX, rect.y() + margin, thick, thick), icon.pixmap(thick, thick, QIcon::Normal));
            return;
        }
    }

    QHeaderView::paintSection(painter, rect, logicalIndex);
}

}
}
}

