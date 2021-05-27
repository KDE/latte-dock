/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
void drawFormattedText(QPainter *painter, const QStyleOptionMenuItem &option, const float textOpacity = 1.0);
void drawFormattedText(QPainter *painter, const QStyleOption &option, const QString &text, const bool &isTextCentered = false, const float textOpacity = 1.0);

//! background
void drawBackground(QPainter *painter, const QStyleOptionViewItem &option);
void drawBackground(QPainter *painter, const QStyle *style, const QStyleOptionMenuItem &option);

//! simple icon
QRect remainedFromIcon(const QStyleOption &option, Qt::AlignmentFlag alignment = Qt::AlignLeft, int lengthMargin = -1, int thickMargin = -1);
void drawIcon(QPainter *painter, const QStyleOption &option, const QString &icon, Qt::AlignmentFlag alignment = Qt::AlignLeft, int lengthMargin = -1, int thickMargin = -1);

//! layout icon
QRect remainedFromLayoutIcon(const QStyleOption &option, Qt::AlignmentFlag alignment = Qt::AlignLeft, int lengthMargin = -1, int thickMargin = -1);
void drawLayoutIcon(QPainter *painter, const QStyleOption &option, const bool &isBackgroundFile, const QString &iconName, Qt::AlignmentFlag alignment = Qt::AlignLeft, int lengthMargin = -1, int thickMargin = -1);

//! changes indicator
QRect remainedFromChangesIndicator(const QStyleOptionViewItem &option);
void drawChangesIndicator(QPainter *painter, const QStyleOptionViewItem &option);


//! screen icon
QRect remainedFromScreenDrawing(const QStyleOption &option, const int &maxIconSize = -1);
QRect drawScreen(QPainter *painter, const QStyleOption &option, QRect screenGeometry, const int &maxIconSize = -1, const float brushOpacity = 1.0); // returns screen available rect

int screenMaxLength(const QStyleOption &option, const int &maxIconSize = -1);

}

#endif
