/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checkboxdelegate.h"

// local
#include "../screensmodel.h"
#include "../../generic/generictools.h"
#include "../../../data/screendata.h"

// Qt
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyleOptionButton>

namespace Latte {
namespace Settings {
namespace Screens{
namespace Delegate {

CheckBox::CheckBox(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

bool CheckBox::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                           const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

    if (event->type() == QEvent::MouseButtonDblClick) {
        if (!option.rect.contains(static_cast<QMouseEvent *>(event)->pos()))
            return false;
    } else if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent *>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent *>(event)->key() != Qt::Key_Select)
            return false;
    } else {
        return false;
    }

    const bool currentState = index.data(Qt::CheckStateRole).toBool();
    return model->setData(index, !currentState, Qt::CheckStateRole);
}

void CheckBox::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem adjustedOption = option;
    //! Remove the focus dotted lines
    adjustedOption.state = (adjustedOption.state & ~QStyle::State_HasFocus);
    adjustedOption.displayAlignment = Qt::AlignLeft;

    bool originalChecked{false};
    bool currentChecked = index.data(Qt::UserRole).toBool();

    QString screendisplay = index.data(Qt::DisplayRole).toString();
    Latte::Data::Screen screen = index.data(Model::Screens::SCREENDATAROLE).value<Latte::Data::Screen>();

    bool isActive = index.data(Model::Screens::ISSCREENACTIVEROLE).toBool();
    bool isSelected = index.data(Model::Screens::ISSELECTEDROLE).toBool();


    //! background
    Latte::drawBackground(painter, adjustedOption);

    //! checkbox
    QStyleOptionButton checkopt;
    checkopt.state |= isSelected ? QStyle::State_On : QStyle::State_Off;
    checkopt.state |= QStyle::State_Enabled;
    checkopt.text = "";
    checkopt.rect = option.rect;

    QRect remainedrect = Latte::remainedFromCheckBox(checkopt);
    Latte::drawCheckBox(painter, checkopt);
    adjustedOption.rect = remainedrect;

    //! screen
    int maxiconsize = -1; //disabled
    remainedrect = Latte::remainedFromScreenDrawing(adjustedOption, maxiconsize);
    Latte::drawScreen(painter, adjustedOption, screen.geometry, maxiconsize);
    adjustedOption.rect = remainedrect;

    //! screen name
    adjustedOption.text = screen.name;

    if (isActive) {
        adjustedOption.text = "<b>" + adjustedOption.text + "</b>";
    }

    Latte::drawFormattedText(painter, adjustedOption);

    //! screen id
    adjustedOption.text = "{" + screen.id + "}";

    if (isActive) {
        adjustedOption.text = "<b>" + adjustedOption.text + "</b>";
    }

    adjustedOption.displayAlignment = Qt::AlignRight;
    Latte::drawFormattedText(painter, adjustedOption);
}


}
}
}
}
