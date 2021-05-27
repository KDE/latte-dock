/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
