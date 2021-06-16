/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "schemescombobox.h"

// local
#include "../generic/generictools.h"

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

SchemesComboBox::SchemesComboBox(QWidget *parent)
    : QComboBox (parent)
{
}

QColor SchemesComboBox::backgroundColor() const
{
    return m_backgroundColor;
}

void SchemesComboBox::setBackgroundColor(const QColor &color)
{
    if (m_backgroundColor == color) {
        return;
    }

    m_backgroundColor = color;
    update();
}

QColor SchemesComboBox::textColor() const
{
    return m_textColor;
}

void SchemesComboBox::setTextColor(const QColor &color)
{
    if (m_textColor == color) {
        return;
    }

    m_textColor = color;
    update();
}


void SchemesComboBox::paintEvent(QPaintEvent *event)
{
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // draw the combobox frame, focusrect and selected etc.
    QStyleOptionComboBox opt;
    initStyleOption(&opt);

    // background
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    // icon
    QRect remained = Latte::remainedFromColorSchemeIcon(opt, Qt::AlignLeft, 3, 5);
    Latte::drawColorSchemeIcon(&painter, opt, m_textColor, m_backgroundColor, Qt::AlignLeft, 7, 6);
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
