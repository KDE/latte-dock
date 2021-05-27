/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layouticondata.h"


namespace Latte {
namespace Data {

LayoutIcon::LayoutIcon()
    : Generic()
{
}

LayoutIcon::LayoutIcon(LayoutIcon &&o)
    : Generic(o),
      isBackgroundFile(o.isBackgroundFile)
{
}

LayoutIcon::LayoutIcon(const LayoutIcon &o)
    : Generic(o),
      isBackgroundFile(o.isBackgroundFile)
{
}

bool LayoutIcon::isEmpty() const
{
    return (id.isEmpty() && name.isEmpty());
}

LayoutIcon &LayoutIcon::operator=(LayoutIcon &&rhs)
{
    id = rhs.id;
    isBackgroundFile = rhs.isBackgroundFile;
    name = rhs.name;

    return (*this);
}

LayoutIcon &LayoutIcon::operator=(const LayoutIcon &rhs)
{
    id = rhs.id;
    isBackgroundFile = rhs.isBackgroundFile;
    name = rhs.name;

    return (*this);
}

bool LayoutIcon::operator==(const LayoutIcon &rhs) const
{
    return (id == rhs.id)
            && (name == rhs.name)
            && (isBackgroundFile == rhs.isBackgroundFile);
}

bool LayoutIcon::operator!=(const LayoutIcon &rhs) const
{
    return !(*this == rhs);
}

}
}

