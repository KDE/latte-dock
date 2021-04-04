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

#include "genericdata.h"

namespace Latte {
namespace Data {

Generic::Generic(Generic &&o)
    : id(o.id),
      name(o.name)
{
}

Generic::Generic(const Generic &o)
    : id(o.id),
      name(o.name)
{
}

Generic::Generic(const QString &newid, const QString &newname)
    : id(newid),
      name(newname)
{
}

Generic &Generic::operator=(const Generic &rhs)
{
    id = rhs.id;
    name = rhs.name;

    return (*this);
}

Generic &Generic::operator=(Generic &&rhs)
{
    id = rhs.id;
    name = rhs.name;

    return (*this);
}

bool Generic::operator==(const Generic &rhs) const
{
    return (id == rhs.id)
            && (name == rhs.name);
}

bool Generic::operator!=(const Generic &rhs) const
{
    return !(*this == rhs);
}

}
}
