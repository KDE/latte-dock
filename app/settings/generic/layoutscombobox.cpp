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

#include "layoutscombobox.h"

// local
#include "generictools.h"

// Qt
#include <QDebug>
#include <QPalette>
#include <QStyleOptionComboBox>
#include <QStylePainter>

namespace Latte {
namespace Settings {

const int MARGIN = 4;
const int VERTMARGIN = 4;

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
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    //! Adjust text and layout icon accordingly
    int thick = opt.rect.height() - 2 * VERTMARGIN;
    int textX = opt.rect.x() + thick + 1;
    QRect textRect(textX, opt.rect.y(), opt.rect.width() - thick - 1, opt.rect.height());

    QStyleOptionComboBox adjOpt = opt;
    adjOpt.rect = textRect;
    // draw text
    painter.drawControl(QStyle::CE_ComboBoxLabel, adjOpt);

    QRect iconRect(opt.rect.x() + MARGIN, opt.rect.y() + VERTMARGIN, thick, thick);
    // draw layout icon
    Latte::drawLayoutIcon(&painter, opt, iconRect,  m_layoutIcon.isBackgroundFile, m_layoutIcon.name);
}


}
}
