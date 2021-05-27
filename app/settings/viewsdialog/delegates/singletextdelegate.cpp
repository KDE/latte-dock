/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

    Latte::drawBackground(painter, option);
    Latte::drawFormattedText(painter, myOptions, textopacity);
}

}
}
}
}
