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

#ifndef GENERICSETTINGSTOOLS_H
#define GENERICSETTINGSTOOLS_H

// local
#include "../../data/layouticondata.h"
#include "../../data/viewdata.h"

// Qt
#include <QPainter>
#include <QPalette>
#include <QRect>
#include <QStyleOption>
#include <QStyleOptionViewItem>

namespace Latte {

bool isActive(const QStyleOption &option);
bool isEnabled(const QStyleOption &option);
bool isFocused(const QStyleOption &option);
bool isSelected(const QStyleOption &option);
bool isHovered(const QStyleOption &option);
bool isTextCentered(const QStyleOptionViewItem &option);

QPalette::ColorGroup colorGroup(const QStyleOption &option);

//! strings that even though they were initially at original list
//! now they are not present to current list
QStringList subtracted(const QStringList &original, const QStringList &current);

void drawFormattedText(QPainter *painter, const QStyleOptionViewItem &option);
void drawLayoutIcon(QPainter *painter, const QStyleOption &option, const QRect &target, const Latte::Data::LayoutIcon &icon);

//! simple icon
QRect remainedFromIcon(const QStyleOption &option);
void drawIconBackground(QPainter *painter, const QStyle *style, const QStyleOptionMenuItem &option);
void drawIcon(QPainter *painter, const QStyleOption &option, const QString &icon);

//! changes indicator
QRect remainedFromChangesIndicator(const QStyleOptionViewItem &option);
void drawChangesIndicatorBackground(QPainter *painter, const QStyleOptionViewItem &option);
void drawChangesIndicator(QPainter *painter, const QStyleOptionViewItem &option);


//! screen icon
QRect remainedFromScreenDrawing(const QStyleOption &option);
QRect drawScreen(QPainter *painter, const QStyleOption &option, QRect screenGeometry); // returns screen available rect
void drawScreenBackground(QPainter *painter, const QStyle *style, const QStyleOptionViewItem &option);
void drawScreenBackground(QPainter *painter, const QStyle *style, const QStyleOptionMenuItem &option);
void drawView(QPainter *painter, const QStyleOptionViewItem &option, const Latte::Data::View &view, const QRect &availableScreenRect);

int screenMaxLength(const QStyleOption &option);

}

#endif
