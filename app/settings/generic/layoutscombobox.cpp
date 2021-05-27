/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layoutscombobox.h"

// local
#include "generictools.h"

// Qt
#include <QApplication>
#include <QDebug>
#include <QPalette>
#include <QStyleOptionComboBox>
#include <QStylePainter>

namespace Latte {
namespace Settings {

const int MARGIN = 2;
const int VERTMARGIN = 3;

LayoutsComboBox::LayoutsComboBox(QWidget *parent)
    : QComboBox (parent)
{
}

Latte::Data::LayoutIcon LayoutsComboBox::layoutIcon() const
{
    return m_layoutIcon;
}

void LayoutsComboBox::setLayoutIcon(const Latte::Data::LayoutIcon &icon)
{
    if (m_layoutIcon == icon) {
        return;
    }

    m_layoutIcon = icon;
    update();
}

void LayoutsComboBox::paintEvent(QPaintEvent *event)
{
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // draw the combobox frame, focusrect and selected etc.
    QStyleOptionComboBox opt;
    initStyleOption(&opt);

    // background
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    // icon
    QRect remained = Latte::remainedFromLayoutIcon(opt, Qt::AlignLeft, 3, 5);
    Latte::drawLayoutIcon(&painter, opt, m_layoutIcon.isBackgroundFile, m_layoutIcon.name, Qt::AlignLeft, 7, 6);
    opt.rect = remained;

    // adjust text place, move it a bit to the left
    QRect textRect;
    int textnegativepad = MARGIN + 1;
    if (qApp->layoutDirection() == Qt::LeftToRight) {
        textRect = QRect(remained.x() - textnegativepad, opt.rect.y(), remained.width() + 2*textnegativepad, opt.rect.height());
    } else {
        textRect = QRect(remained.x(), opt.rect.y(), remained.width() + 2 * textnegativepad, opt.rect.height());
    }
    opt.rect = textRect;

    // text
    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);

}


}
}
