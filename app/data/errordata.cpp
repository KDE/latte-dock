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

#include "errordata.h"

namespace Latte {
namespace Data {

Error::Error()
    : Generic()
{
}

Error::Error(Error &&o)
    : Generic(o),
      information(o.information)
{
}

Error::Error(const Error &o)
    : Generic(o),
      information(o.information)
{
}

Error &Error::operator=(const Error &rhs)
{
    id = rhs.id;
    name = rhs.name;
    information = rhs.information;

    return (*this);
}

Error &Error::operator=(Error &&rhs)
{
    id = rhs.id;
    name = rhs.name;
    information = rhs.information;

    return (*this);
}

bool Error::operator==(const Error &rhs) const
{
    return (id == rhs.id)
            && (name == rhs.name)
            && (information == rhs.information);
}

bool Error::operator!=(const Error &rhs) const
{
    return !(*this == rhs);
}

bool Error::isValid() const
{
    return !id.isEmpty();
}

}
}
