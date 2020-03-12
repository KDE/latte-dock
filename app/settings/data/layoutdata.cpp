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

#include "layoutdata.h"

namespace Latte {
namespace Settings {
namespace Data {

Layout::Layout()
{
}

Layout::~Layout()
{
}

Layout::Layout(const Layout &o)
    : id(o.id),
      name(o.name),
      background(o.background),
      backgroundTextColor(o.backgroundTextColor),
      isActive(o.isActive),
      isLocked(o.isLocked),
      isShownInMenu(o.isShownInMenu),
      hasDisabledBorders(o.hasDisabledBorders),
      activities(o.activities),
      shares(o.shares)
{
}

Layout &Layout::operator=(const Layout &rhs)
{
    id = rhs.id;
    name = rhs.name;
    background = rhs.background;
    backgroundTextColor = rhs.backgroundTextColor;
    isActive = rhs.isActive;
    isLocked = rhs.isLocked;
    isShownInMenu = rhs.isShownInMenu;
    hasDisabledBorders = rhs.hasDisabledBorders;
    activities = rhs.activities;
    shares = rhs.shares;

    return *this;
}

bool Layout::operator==(const Layout &rhs) const
{
    return (id == rhs.id)
            && (name == rhs.name)
            && (background == rhs.background)
            && (backgroundTextColor == rhs.backgroundTextColor)
            && (isActive == rhs.isActive)
            && (isLocked == rhs.isLocked)
            && (isShownInMenu == rhs.isShownInMenu)
            && (hasDisabledBorders == rhs.hasDisabledBorders)
            && (activities == rhs.activities)
            && (shares == rhs.shares);
}

bool Layout::operator!=(const Layout &rhs) const
{
    return !(*this == rhs);
}

bool Layout::isShared() const
{
    return !shares.isEmpty();
}

}
}
}

