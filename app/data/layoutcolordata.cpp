/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "layoutcolordata.h"

namespace Latte {
namespace Data {

LayoutColor::LayoutColor()
{
}

LayoutColor::LayoutColor(LayoutColor &&o)
    : id(o.id),
      name(o.name),
      path(o.path),
      textColor(o.textColor)
{
}

LayoutColor::LayoutColor(const LayoutColor &o)
    : id(o.id),
      name(o.name),
      path(o.path),
      textColor(o.textColor)
{
}

LayoutColor &LayoutColor::operator=(const LayoutColor &rhs)
{
    id = rhs.id;
    name = rhs.name;
    path = rhs.path;
    textColor = rhs.textColor;

    return (*this);
}

LayoutColor &LayoutColor::operator=(LayoutColor &&rhs)
{
    id = rhs.id;
    name = rhs.name;
    path = rhs.path;
    textColor = rhs.textColor;

    return (*this);
}

bool LayoutColor::operator==(const LayoutColor &rhs) const
{
    return (id == rhs.id)
            && (name == rhs.name)
            && (path == rhs.path)
            && (textColor == rhs.textColor);
}

bool LayoutColor::operator!=(const LayoutColor &rhs) const
{
    return !(*this == rhs);
}

void LayoutColor::setData(const QString &newid, const QString &newname, const QString &newpath, const QString &newtextcolor)
{
    id = newid;
    name = newname;
    path = newpath;
    textColor = newtextcolor;
}

}
}
