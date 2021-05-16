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
    Latte::Data::LayoutIcon icon = index.data(Qt::UserRole).value<Latte::Data::LayoutIcon>();

    //! background
    Latte::drawBackground(painter, option);
    Latte::drawLayoutIcon(painter, option, icon.isBackgroundFile, icon.name, Qt::AlignHCenter, -1, 2);
}

}
}
}
}

