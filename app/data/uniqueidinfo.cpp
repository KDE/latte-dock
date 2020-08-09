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

#include "uniqueidinfo.h"

namespace Latte {
namespace Settings {
namespace Data {

UniqueIdInfo::UniqueIdInfo()
{
}

UniqueIdInfo::UniqueIdInfo(UniqueIdInfo &&o)
    : newId(o.newId),
      newName(o.newName),
      oldId(o.oldId),
      oldName(o.oldName)
{
}

UniqueIdInfo::UniqueIdInfo(const UniqueIdInfo &o)
    : newId(o.newId),
      newName(o.newName),
      oldId(o.oldId),
      oldName(o.oldName)
{
}

UniqueIdInfo &UniqueIdInfo::operator=(const UniqueIdInfo &rhs)
{
    newId = rhs.newId;
    newName = rhs.newName;
    oldId = rhs.oldId;
    oldName = rhs.oldName;

    return (*this);
}

UniqueIdInfo &UniqueIdInfo::operator=(UniqueIdInfo &&rhs)
{
    newId = rhs.newId;
    newName = rhs.newName;
    oldId = rhs.oldId;
    oldName = rhs.oldName;

    return (*this);
}

bool UniqueIdInfo::operator==(const UniqueIdInfo &rhs) const
{
    return (newId == rhs.newId)
            && (newName == rhs.newName)
            && (oldId == rhs.oldId)
            && (oldName == rhs.oldName);
}

bool UniqueIdInfo::operator!=(const UniqueIdInfo &rhs) const
{
    return !(*this == rhs);
}

}
}
}
