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

void drawFormattedText(QPainter *painter, const QStyleOptionViewItem &option, const float textOpacity = 1.0);

//! background
void drawBackground(QPainter *painter, const QStyleOptionViewItem &option);
void drawBackground(QPainter *painter, const QStyle *style, const QStyleOptionMenuItem &option);

//! simple icon
QRect remainedFromIcon(const QStyleOption &option, Qt::AlignmentFlag alignment = Qt::AlignLeft);
void drawIcon(QPainter *painter, const QStyleOption &option, const QString &icon, Qt::AlignmentFlag alignment = Qt::AlignLeft);

//! layout icon
QRect remainedFromLayoutIcon(const QStyleOption &option, Qt::AlignmentFlag alignment = Qt::AlignLeft);
void drawLayoutIcon(QPainter *painter, const QStyleOption &option, const bool &isBackgroundFile, const QString &iconName, Qt::AlignmentFlag alignment = Qt::AlignLeft);

//! changes indicator
QRect remainedFromChangesIndicator(const QStyleOptionViewItem &option);
void drawChangesIndicator(QPainter *painter, const QStyleOptionViewItem &option);


//! screen icon
QRect remainedFromScreenDrawing(const QStyleOption &option, const int &maxIconSize = -1);
QRect drawScreen(QPainter *painter, const QStyleOption &option, QRect screenGeometry, const int &maxIconSize = -1, const float brushOpacity = 1.0); // returns screen available rect

int screenMaxLength(const QStyleOption &option, const int &maxIconSize = -1);

}

#endif
