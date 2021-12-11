/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "namedelegate.h"

// local
#include "../viewsmodel.h"
#include "../../generic/generictools.h"
#include "../../generic/genericviewtools.h"
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
    editor->setGeometry(Latte::remainedFromScreenDrawing(option, false));
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
        remainedrect = Latte::remainedFromIcon(myOptions, Qt::AlignRight, -1, 2);
        if (hasErrors) {
            Latte::drawIcon(painter, myOptions, "data-error", Qt::AlignRight, -1, 2);
        } else if (hasWarnings) {
            Latte::drawIcon(painter, myOptions, "data-warning", Qt::AlignRight, -1, 2);
        }
        myOptions.rect = remainedrect;
    }

    // draw screen icon
    int maxiconsize = -1; //disabled
    remainedrect = Latte::remainedFromScreenDrawing(myOptions, screen.isScreensGroup(), maxiconsize);
    QRect availableScreenRect = Latte::drawScreen(painter, myOptions, screen.isScreensGroup(), screen.geometry, maxiconsize, textopacity);
    Latte::drawView(painter, myOptions, view, availableScreenRect, textopacity);

    myOptions.rect = remainedrect;
    Latte::drawFormattedText(painter, myOptions, textopacity);
}

}
}
}
}
