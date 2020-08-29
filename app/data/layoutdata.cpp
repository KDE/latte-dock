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

// Qt
#include <QDir>

namespace Latte {
namespace Data {

Layout::Layout()
    : Generic()
{
}

Layout::Layout(Layout &&o)
    : Generic(o),
      icon(o.icon),
      color(o.color),
      background(o.background),
      textColor(o.textColor),
      isActive(o.isActive),
      isBroken(o.isBroken),
      isLocked(o.isLocked),
      isShownInMenu(o.isShownInMenu),
      isTemplate(o.isTemplate),
      hasDisabledBorders(o.hasDisabledBorders),
      activities(o.activities),
      backgroundStyle(o.backgroundStyle)
{
}

Layout::Layout(const Layout &o)
    : Generic(o),
      icon(o.icon),
      color(o.color),
      background(o.background),
      textColor(o.textColor),
      isActive(o.isActive),
      isBroken(o.isBroken),
      isLocked(o.isLocked),
      isShownInMenu(o.isShownInMenu),
      isTemplate(o.isTemplate),
      hasDisabledBorders(o.hasDisabledBorders),
      activities(o.activities),
      backgroundStyle(o.backgroundStyle)
{
}

Layout &Layout::operator=(Layout &&rhs)
{
    id = rhs.id;
    name = rhs.name;
    icon = rhs.icon;
    color = rhs.color;
    background = rhs.background;
    textColor = rhs.textColor;
    isActive = rhs.isActive;
    isBroken = rhs.isBroken;
    isLocked = rhs.isLocked;
    isShownInMenu = rhs.isShownInMenu;
    isTemplate = rhs.isTemplate;
    hasDisabledBorders = rhs.hasDisabledBorders;
    activities = rhs.activities;
    backgroundStyle = rhs.backgroundStyle;

    return (*this);
}

Layout &Layout::operator=(const Layout &rhs)
{
    id = rhs.id;
    name = rhs.name;
    icon = rhs.icon;
    color = rhs.color;
    background = rhs.background;
    textColor = rhs.textColor;
    isActive = rhs.isActive;
    isBroken = rhs.isBroken;
    isLocked = rhs.isLocked;
    isShownInMenu = rhs.isShownInMenu;
    isTemplate = rhs.isTemplate;
    hasDisabledBorders = rhs.hasDisabledBorders;
    activities = rhs.activities;
    backgroundStyle = rhs.backgroundStyle;

    return (*this);
}

bool Layout::operator==(const Layout &rhs) const
{
    return (id == rhs.id)
            && (name == rhs.name)
            && (icon == rhs.icon)
            && (color == rhs.color)
            && (background == rhs.background)
            && (textColor == rhs.textColor)
            && (isBroken == rhs.isBroken)
            //&& (isActive == rhs.isActive) /*Disabled but this is not a data but a layout state*/
            && (isLocked == rhs.isLocked)
            && (isShownInMenu == rhs.isShownInMenu)
            && (isTemplate == rhs.isTemplate)
            && (hasDisabledBorders == rhs.hasDisabledBorders)
            && (activities == rhs.activities)
            && (backgroundStyle == rhs.backgroundStyle);
}

bool Layout::operator!=(const Layout &rhs) const
{
    return !(*this == rhs);
}

bool Layout::isOnAllActivities() const
{
    return ((activities.count() == 1) && (activities[0] == ALLACTIVITIESID));
}

bool Layout::isForFreeActivities() const
{
    return ((activities.count() == 1) && (activities[0] == FREEACTIVITIESID));
}

bool Layout::isTemporary() const
{
    return id.startsWith("/tmp");
}

bool Layout::isEmpty() const
{
    return isNull();
}

bool Layout::isNull() const
{
    return (id.isEmpty() && name.isEmpty());
}

bool Layout::isSystemTemplate() const
{
    return isTemplate && !id.startsWith(QDir::tempPath()) && !id.startsWith(QDir::homePath());
}

}
}

