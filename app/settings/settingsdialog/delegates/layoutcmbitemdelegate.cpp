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

#include "layoutcmbitemdelegate.h"

// local
#include "../layoutsmodel.h"
#include "../../generic/generictools.h"

// Qt
#include <QDebug>
#include <QModelIndex>
#include <QPainter>
#include <QString>


namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

const int MARGIN = 1;

LayoutCmbItemDelegate::LayoutCmbItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void LayoutCmbItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOptions = option;
    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);

    //! draw underlying background
    QStyledItemDelegate::paint(painter, myOptions, index.model()->index(index.row(), Model::Layouts::HIDDENTEXTCOLUMN));

    QList<Latte::Data::LayoutIcon> icons = index.data(Model::Layouts::BACKGROUNDUSERROLE).value<QList<Latte::Data::LayoutIcon>>();

    int iconsLength = (option.rect.height() + 4 * MARGIN);

    if (icons.count() > 0) {
        int localMargin = MARGIN-1;

        int aY = option.rect.y() + localMargin;
        int thick = option.rect.height() - localMargin*2;

        int centerX = option.rect.x() + iconsLength / 2;
        int step = thick;
        int total_icons_width = (thick-step) + icons.count() * step;

        if (total_icons_width > option.rect.width()){
            step = thick/2;
            total_icons_width = (thick-step) + icons.count() * step;
        }

        int startX = centerX - (total_icons_width/2);

        for (int i=0; i<icons.count(); ++i) {
            int tX = startX + (i * step);
            Latte::drawLayoutIcon(painter, option, QRect(tX, aY, thick, thick), icons[i]);
        }
    }

    myOptions.rect.setWidth(option.rect.width() - iconsLength);
    myOptions.rect.moveLeft(option.rect.x() + iconsLength);
    QStyledItemDelegate::paint(painter, myOptions, index);
}

}
}
}
}

