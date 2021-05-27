/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "uniqueidinfo.h"

namespace Latte {
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
