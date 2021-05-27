/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layoutcolordata.h"

namespace Latte {
namespace Data {

LayoutColor::LayoutColor()
    : Generic()
{
}

LayoutColor::LayoutColor(LayoutColor &&o)
    : Generic(o),
      path(o.path),
      textColor(o.textColor)
{
}

LayoutColor::LayoutColor(const LayoutColor &o)
    : Generic(o),
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
    return  (id == rhs.id)
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
