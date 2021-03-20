/*
 * Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "viewdata.h"

namespace Latte {
namespace Data {

View::View()
    : Generic()
{
}

View::View(View &&o)
    : Generic(o),
      onPrimary(o.onPrimary),
      screen(o.screen),
      maxLength(o.maxLength),
      alignment(o.alignment)
{
}

View::View(const View &o)
    : Generic(o),
      onPrimary(o.onPrimary),
      screen(o.screen),
      maxLength(o.maxLength),
      alignment(o.alignment)
{
}

View &View::operator=(const View &rhs)
{
    id = rhs.id;
    name = rhs.name;
    onPrimary = rhs.onPrimary;
    screen = rhs.screen;
    maxLength = rhs.maxLength;
    alignment = rhs.alignment;

    return (*this);
}

View &View::operator=(View &&rhs)
{
    id = rhs.id;
    name = rhs.name;
    onPrimary = rhs.onPrimary;
    screen = rhs.screen;
    maxLength = rhs.maxLength;
    alignment = rhs.alignment;

    return (*this);
}

}
}
