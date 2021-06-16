/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "schemecmbitemdelegate.h"

// local
#include "../schemesmodel.h"
#include "../../generic/generictools.h"

// Qt
#include <QColor>
#include <QDebug>
#include <QModelIndex>
#include <QPainter>
#include <QString>


namespace Latte {
namespace Settings {
namespace Details {
namespace Delegate {

SchemeCmbItemDelegate::SchemeCmbItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void SchemeCmbItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOptions = option;

    //! background
    Latte::drawBackground(painter, option);

    QColor backcolor = index.data(Model::Schemes::BACKGROUNDCOLORROLE).value<QColor>();
    QColor textcolor = index.data(Model::Schemes::TEXTCOLORROLE).value<QColor>();

    //! icon
    QRect remained = Latte::remainedFromColorSchemeIcon(myOptions, Qt::AlignLeft, 4, 2);
    Latte::drawColorSchemeIcon(painter, myOptions, textcolor, backcolor, Qt::AlignLeft, 5, 2); //+1px in order to take into account popup window border
    myOptions.rect = remained;

    //!
    QStyledItemDelegate::paint(painter, myOptions, index);
}

}
}
}
}

