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

#include "backgrounddelegate.h"

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

const int MARGIN = 2;

BackgroundDelegate::BackgroundDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void BackgroundDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOptions = option;
    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);

    //! background
    Latte::drawBackground(painter, option);

    Latte::Data::LayoutIcon icon = index.data(Qt::UserRole).value<Latte::Data::LayoutIcon>();

    if (!icon.isEmpty()) {
        int localMargin = MARGIN-1;// icons[0].isBackgroundFile && icons.count() == 1 ? qMin(option.rect.height()/4,MARGIN+5) : MARGIN-1;

        int aY = option.rect.y() + localMargin;
        int thick = option.rect.height() - localMargin*2;

        int centerX = option.rect.x() + (option.rect.width() / 2);
        int step = thick;
        int total_icons_width = (thick-step) + step;

        if (total_icons_width > option.rect.width()){
            step = thick/2;
            total_icons_width = (thick-step) + step;
        }

        int startX = centerX - (total_icons_width/2);

        Latte::drawLayoutIcon(painter, option, QRect(startX, aY, thick, thick), icon.isBackgroundFile, icon.name);
    }
}

}
}
}
}

