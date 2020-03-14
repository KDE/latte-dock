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

Layout::Layout(Layout &&o)
    : id(o.id),
      m_currentName(o.m_currentName),
      m_originalName(o.m_originalName),
      color(o.color),
      background(o.background),
      textColor(o.textColor),
      isActive(o.isActive),
      isLocked(o.isLocked),
      isShownInMenu(o.isShownInMenu),
      hasDisabledBorders(o.hasDisabledBorders),
      activities(o.activities),
      shares(o.shares)
{
}

Layout::Layout(const Layout &o)
    : id(o.id),
      m_currentName(o.m_currentName),
      m_originalName(o.m_originalName),
      color(o.color),
      background(o.background),
      textColor(o.textColor),
      isActive(o.isActive),
      isLocked(o.isLocked),
      isShownInMenu(o.isShownInMenu),
      hasDisabledBorders(o.hasDisabledBorders),
      activities(o.activities),
      shares(o.shares)
{
}

Layout &Layout::operator=(Layout &&rhs)
{
    id = rhs.id;
    m_currentName = rhs.m_currentName;
    m_originalName = rhs.m_originalName;
    color = rhs.color;
    background = rhs.background;
    textColor = rhs.textColor;
    isActive = rhs.isActive;
    isLocked = rhs.isLocked;
    isShownInMenu = rhs.isShownInMenu;
    hasDisabledBorders = rhs.hasDisabledBorders;
    activities = rhs.activities;
    shares = rhs.shares;

    return (*this);
}

Layout &Layout::operator=(const Layout &rhs)
{
    id = rhs.id;
    m_currentName = rhs.m_currentName;
    m_originalName = rhs.m_originalName;
    color = rhs.color;
    background = rhs.background;
    textColor = rhs.textColor;
    isActive = rhs.isActive;
    isLocked = rhs.isLocked;
    isShownInMenu = rhs.isShownInMenu;
    hasDisabledBorders = rhs.hasDisabledBorders;
    activities = rhs.activities;
    shares = rhs.shares;

    return (*this);
}

bool Layout::operator==(const Layout &rhs) const
{
    return (id == rhs.id)
            && (m_currentName == rhs.m_currentName)
            && (m_originalName == rhs.m_originalName)
            && (color == rhs.color)
            && (background == rhs.background)
            && (textColor == rhs.textColor)
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

bool Layout::nameWasEdited() const
{
    return (!m_currentName.isEmpty() && (m_currentName != m_originalName)) || id.startsWith("/tmp");
}

QString Layout::currentName() const
{
    return m_currentName.isEmpty() ? m_originalName : m_currentName;
}

void Layout::setCurrentName(const QString name)
{
    m_currentName = name;
}

QString Layout::originalName() const
{
    return m_originalName;
}

void Layout::setOriginalName(const QString name)
{
    m_originalName = name;
    m_currentName = "";
}

}
}
}

