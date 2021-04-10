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

#include "namedelegate.h"

// local
#include "../viewsmodel.h"
#include "../../generic/generictools.h"

// KDE
#include <KLocalizedString>

namespace Latte {
namespace Settings {
namespace View {
namespace Delegate {

NameDelegate::NameDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void NameDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOptions = option;
    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);
    myOptions.text = index.model()->data(index, Qt::DisplayRole).toString();
    myOptions.displayAlignment = static_cast<Qt::Alignment>(index.model()->data(index, Qt::TextAlignmentRole).toInt());

    bool isEmpty = myOptions.text.isEmpty();
    bool isActive = index.data(Model::Views::ISACTIVEROLE).toBool();
    bool isChanged = (index.data(Model::Views::ISCHANGEDROLE).toBool() || index.data(Model::Views::HASCHANGEDVIEWROLE).toBool());

    if (isEmpty) {
        myOptions.displayAlignment = (Qt::AlignHCenter | Qt::AlignVCenter);
        myOptions.text = " &lt;" + i18n("optional") + "&gt; ";
       // QBrush placeholderBrush = option.palette.placeholderText();
        //style="color:blue;"
        QPalette::ColorRole applyColor = Latte::isSelected(option) ? QPalette::HighlightedText : QPalette::Text;
        QBrush placeholderBrush = option.palette.brush(Latte::colorGroup(option), applyColor);
        QColor placeholderColor = placeholderBrush.color();

        placeholderColor.setAlpha(125);

        QString cssplaceholdercolor = "rgba(";
        cssplaceholdercolor += QString::number(placeholderColor.red()) + ",";
        cssplaceholdercolor += QString::number(placeholderColor.green()) + ", ";
        cssplaceholdercolor += QString::number(placeholderColor.blue()) + ", ";
        cssplaceholdercolor += "110)";

        myOptions.text = "<label style='color:" + cssplaceholdercolor + ";'>" + myOptions.text + "</label>";

        qDebug() << "org.kde.latte ::  " <<myOptions.text;
    }

    if (isActive) {
        myOptions.text = "<b>" + myOptions.text + "</b>";
    }

    if (isChanged) {
        myOptions.text = "<i>" + myOptions.text + "</i>";
    }

    Latte::drawFormattedText(painter, myOptions);
}

}
}
}
}
