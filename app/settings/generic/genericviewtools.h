/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef GENERICVIEWTOOLS_H
#define GENERICVIEWTOOLS_H

// local
#include "../../data/viewdata.h"

// Qt
#include <QPainter>
#include <QPalette>
#include <QRect>
#include <QStyleOption>
#include <QStyleOptionViewItem>

namespace Latte {

void drawView(QPainter *painter, const QStyleOption &option, const Latte::Data::View &view, const QRect &availableScreenRect, const float brushOpacity = 1.0);

}

#endif
