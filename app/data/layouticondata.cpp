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

#include "layouticondata.h"


namespace Latte {
namespace Settings {
namespace Data {

LayoutIcon::LayoutIcon()
{
}

LayoutIcon::LayoutIcon(LayoutIcon &&o)
    : id(o.id),
      isFreeActivities(o.isFreeActivities),
      isBackgroundFile(o.isBackgroundFile),
      name(o.name)
{
}

LayoutIcon::LayoutIcon(const LayoutIcon &o)
    : id(o.id),
      isFreeActivities(o.isFreeActivities),
      isBackgroundFile(o.isBackgroundFile),
      name(o.name)
{
}

LayoutIcon &LayoutIcon::operator=(LayoutIcon &&rhs)
{
    id = rhs.id;
    isFreeActivities = rhs.isFreeActivities;
    isBackgroundFile = rhs.isBackgroundFile;
    name = rhs.name;

    return (*this);
}

LayoutIcon &LayoutIcon::operator=(const LayoutIcon &rhs)
{
    id = rhs.id;
    isFreeActivities = rhs.isFreeActivities;
    isBackgroundFile = rhs.isBackgroundFile;
    name = rhs.name;

    return (*this);
}

bool LayoutIcon::operator==(const LayoutIcon &rhs) const
{
    return (id == rhs.id)
            && (name == rhs.name)
            && (isFreeActivities == rhs.isFreeActivities)
            && (isBackgroundFile == rhs.isBackgroundFile);
}

bool LayoutIcon::operator!=(const LayoutIcon &rhs) const
{
    return !(*this == rhs);
}

}
}
}

