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

#include "singletextdelegate.h"

// local
#include "../viewsmodel.h"
#include "../../generic/generictools.h"

namespace Latte {
namespace Settings {
namespace View {
namespace Delegate {

SingleText::SingleText(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void SingleText::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOptions = option;
    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);
    myOptions.text = index.model()->data(index, Qt::DisplayRole).toString();
    myOptions.displayAlignment = static_cast<Qt::Alignment>(index.model()->data(index, Qt::TextAlignmentRole).toInt());

    bool isActive = index.data(Model::Views::ISACTIVEROLE).toBool();
    bool isMoveOrigin = index.data(Model::Views::ISMOVEORIGINROLE).toBool();
    bool isChanged = isMoveOrigin;

    float textopacity = 1.0;

    if (isActive) {
        myOptions.text = "<b>" + myOptions.text + "</b>";
    }

    if (isMoveOrigin) {
        textopacity = 0.25;
    }

    Latte::drawFormattedText(painter, myOptions, textopacity);
}

}
}
}
}
