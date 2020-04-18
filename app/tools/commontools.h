/*
 * Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef COMMONTOOLS_H
#define COMMONTOOLS_H

// Qt
#include <QColor>

namespace Latte {

float colorBrightness(QColor color);
float colorBrightness(QRgb rgb);
float colorBrightness(float r, float g, float b);

float colorLumina(QColor color);
float colorLumina(QRgb rgb);
float colorLumina(float r, float g, float b);

//! returns the standard path found that contains the subPath
//! local paths have higher priority by default
QString standardPath(QString subPath, bool localFirst = true);
}

#endif
