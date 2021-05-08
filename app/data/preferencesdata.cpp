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

#include "preferencesdata.h"

namespace Latte {
namespace Data {

const bool Preferences::BADGE3DSTYLE;
const bool Preferences::LAYOUTSINFORMATIONWINDOW;
const bool Preferences::AUTOSTART;
const bool Preferences::BORDERLESSMAXIMIZED;
const bool Preferences::METAPRESSFORAPPLAUNCHER;
const bool Preferences::METAHOLDFORBADGES;
const int Preferences::SCREENSDELAY;
const Settings::MouseSensitivity Preferences::MOUSESENSITIVITY;

Preferences::Preferences()
{
}

Preferences::Preferences(Preferences &&o)
    : badgeStyle3D(o.badgeStyle3D),
      layoutsInformationWindow(o.layoutsInformationWindow),
      autostart(o.autostart),
      borderlessMaximized(o.borderlessMaximized),
      metaPressForAppLauncher(o.metaPressForAppLauncher),
      metaHoldForBadges(o.metaHoldForBadges),
      mouseSensitivity(o.mouseSensitivity),
      screensDelay(o.screensDelay)
{
}

Preferences::Preferences(const Preferences &o)
    : badgeStyle3D(o.badgeStyle3D),
      layoutsInformationWindow(o.layoutsInformationWindow),
      autostart(o.autostart),
      borderlessMaximized(o.borderlessMaximized),
      metaPressForAppLauncher(o.metaPressForAppLauncher),
      metaHoldForBadges(o.metaHoldForBadges),
      mouseSensitivity(o.mouseSensitivity),
      screensDelay(o.screensDelay)
{
}

Preferences &Preferences::operator=(const Preferences &rhs)
{
    badgeStyle3D = rhs.badgeStyle3D;
    layoutsInformationWindow = rhs.layoutsInformationWindow;
    autostart = rhs.autostart;
    borderlessMaximized = rhs.borderlessMaximized;
    metaPressForAppLauncher = rhs.metaPressForAppLauncher;
    metaHoldForBadges = rhs.metaHoldForBadges;
    mouseSensitivity = rhs.mouseSensitivity;
    screensDelay = rhs.screensDelay;

    return (*this);
}

Preferences &Preferences::operator=(Preferences &&rhs)
{
    badgeStyle3D = rhs.badgeStyle3D;
    layoutsInformationWindow = rhs.layoutsInformationWindow;
    autostart = rhs.autostart;
    borderlessMaximized = rhs.borderlessMaximized;
    metaPressForAppLauncher = rhs.metaPressForAppLauncher;
    metaHoldForBadges = rhs.metaHoldForBadges;
    mouseSensitivity = rhs.mouseSensitivity;
    screensDelay = rhs.screensDelay;

    return (*this);
}

bool Preferences::operator==(const Preferences &rhs) const
{
    return (badgeStyle3D == rhs.badgeStyle3D)
            && (layoutsInformationWindow == rhs.layoutsInformationWindow)
            && (autostart == rhs.autostart)
            && (borderlessMaximized == rhs.borderlessMaximized)
            && (metaPressForAppLauncher == rhs.metaPressForAppLauncher)
            && (metaHoldForBadges == rhs.metaHoldForBadges)
            && (mouseSensitivity == rhs.mouseSensitivity)
            && (screensDelay == rhs.screensDelay);
}

bool Preferences::operator!=(const Preferences &rhs) const
{
    return !(*this == rhs);
}

bool Preferences::inDefaultValues() const
{
    return (badgeStyle3D == BADGE3DSTYLE)
            && (layoutsInformationWindow == LAYOUTSINFORMATIONWINDOW)
            && (autostart == AUTOSTART)
            && (borderlessMaximized == BORDERLESSMAXIMIZED)
            && (metaPressForAppLauncher == METAPRESSFORAPPLAUNCHER)
            && (metaHoldForBadges == METAHOLDFORBADGES)
            && (mouseSensitivity == MOUSESENSITIVITY)
            && (screensDelay == SCREENSDELAY);
}

void Preferences::setToDefaults()
{
    badgeStyle3D = BADGE3DSTYLE;
    layoutsInformationWindow = LAYOUTSINFORMATIONWINDOW;
    autostart = AUTOSTART;
    borderlessMaximized = BORDERLESSMAXIMIZED;
    metaPressForAppLauncher = METAPRESSFORAPPLAUNCHER;
    metaHoldForBadges = METAHOLDFORBADGES;
    mouseSensitivity = MOUSESENSITIVITY;
    screensDelay = SCREENSDELAY;
}


}
}
