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
#include "../../../data/screendata.h"
#include "../../../data/viewdata.h"

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

void NameDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(Latte::remainedFromScreenDrawing(option));
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
    bool isMoveOrigin = index.data(Model::Views::ISMOVEORIGINROLE).toBool();
    bool isChanged = (index.data(Model::Views::ISCHANGEDROLE).toBool() || index.data(Model::Views::HASCHANGEDVIEWROLE).toBool());

    bool hasErrors = index.data(Model::Views::ERRORSROLE).toBool();
    bool hasWarnings = index.data(Model::Views::WARNINGSROLE).toBool();

    Latte::Data::Screen screen = index.data(Model::Views::SCREENROLE).value<Latte::Data::Screen>();
    Latte::Data::View view = index.data(Model::Views::VIEWROLE).value<Latte::Data::View>();

    float textopacity = 1.0;

    if (isEmpty) {
        myOptions.text = "&lt; " + i18n("optional") + " &gt;";
        textopacity = 0.5;
    }

    if (isActive) {
        myOptions.text = "<b>" + myOptions.text + "</b>";
    }

    if (isChanged || isMoveOrigin) {
        myOptions.text = "<i>" + myOptions.text + "</i>";
    }

    if (isMoveOrigin) {
        textopacity = 0.25;
    }

    Latte::drawBackground(painter, option);

    // draw changes indicator
    QRect remainedrect = Latte::remainedFromChangesIndicator(option);
    if (isChanged) {
        Latte::drawChangesIndicator(painter, option);
    }
    myOptions.rect = remainedrect;

    // draw errors/warnings
    if (hasErrors || hasWarnings) {
        remainedrect = Latte::remainedFromIcon(myOptions, Qt::AlignRight);
        if (hasErrors) {
            Latte::drawIcon(painter, myOptions, "data-error", Qt::AlignRight);
        } else if (hasWarnings) {
            Latte::drawIcon(painter, myOptions, "data-warning", Qt::AlignRight);
        }
        myOptions.rect = remainedrect;
    }

    // draw screen icon
    int maxiconsize = -1; //disabled
    remainedrect = Latte::remainedFromScreenDrawing(myOptions, maxiconsize);
    QRect availableScreenRect = Latte::drawScreen(painter, myOptions, screen.geometry, maxiconsize, textopacity);
    Latte::drawView(painter, myOptions, view, availableScreenRect, textopacity);

    myOptions.rect = remainedrect;
    Latte::drawFormattedText(painter, myOptions, textopacity);
}

}
}
}
}
