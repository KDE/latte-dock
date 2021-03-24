/*
 * Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "generictools.h"

// Qt
#include <QStyle>

namespace Latte {

const int MARGIN = 1;

bool isEnabled(const QStyleOptionViewItem &option)
{
    if (option.state & QStyle::State_Enabled) {
        return true;
    }

    return false;
}

bool isActive(const QStyleOptionViewItem &option)
{
    if (option.state & QStyle::State_Active) {
        return true;
    }

    return false;
}

bool isSelected(const QStyleOptionViewItem &option)
{
    if (option.state & QStyle::State_Selected) {
        return true;
    }

    return false;
}

bool isHovered(const QStyleOptionViewItem &option)
{
    if (option.state & QStyle::State_MouseOver) {
        return true;
    }

    return false;
}

bool isFocused(const QStyleOptionViewItem &option)
{
    if (option.state & QStyle::State_HasFocus) {
        return true;
    }

    return false;
}

QPalette::ColorGroup colorGroup(const QStyleOptionViewItem &option)
{
    if (!isEnabled(option)) {
        return QPalette::Disabled;
    }

    if (isActive(option) || isFocused(option)) {
        return QPalette::Active;
    }

    if (!isActive(option) && isSelected(option)) {
        return QPalette::Inactive;
    }

    return QPalette::Normal;
}

QStringList subtracted(const QStringList &original, const QStringList &current)
{
    QStringList subtract;

    for(int i=0; i<original.count(); ++i) {
        if (!current.contains(original[i])) {
            subtract << original[i];
        }
    }

    return subtract;
}

void drawLayoutIcon(QPainter *painter, const QStyleOptionViewItem &option, const QRect &target, const Latte::Data::LayoutIcon &icon)
{
    bool active = Latte::isActive(option);
    bool selected = Latte::isSelected(option);
    bool focused = Latte::isFocused(option);

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (icon.isBackgroundFile) {
        int backImageMargin = qMin(option.rect.height()/4, MARGIN+2);
        QRect backTarget(target.x() + backImageMargin, target.y() + backImageMargin, target.width() - 2*backImageMargin, target.height() - 2*backImageMargin);

        QPixmap backImage(icon.name);
        backImage = backImage.copy(backTarget);

        QPalette::ColorRole textColorRole = selected ? QPalette::HighlightedText : QPalette::Text;

        QBrush imageBrush(backImage);
        QPen pen; pen.setWidth(1);
        pen.setColor(option.palette.color(Latte::colorGroup(option), textColorRole));

        painter->setBrush(imageBrush);
        painter->setPen(pen);

        painter->drawEllipse(backTarget);
    } else {
        QIcon::Mode mode = ((active && (selected || focused)) ? QIcon::Selected : QIcon::Normal);

        painter->drawPixmap(target, QIcon::fromTheme(icon.name).pixmap(target.height(), target.height(), mode));
    }

}

}
