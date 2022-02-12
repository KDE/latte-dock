/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "preferencesdata.h"

namespace Latte {
namespace Data {

const bool Preferences::BADGE3DSTYLE;
const bool Preferences::LAYOUTSINFORMATIONWINDOW;
const bool Preferences::AUTOSTART;
const bool Preferences::BORDERLESSMAXIMIZED;
const bool Preferences::ISAVAILABLEGEOMETRYBROADCASTEDTOPLASMA;
const bool Preferences::METAPRESSFORAPPLAUNCHER;
const bool Preferences::METAHOLDFORBADGES;
const int Preferences::PARABOLICSPREAD;
const int Preferences::SCREENSDELAY;
const float Preferences::THICKNESSMARGININFLUENCE = 1.0f;
const Settings::MouseSensitivity Preferences::MOUSESENSITIVITY;

Preferences::Preferences()
{
}

Preferences::Preferences(Preferences &&o)
    : badgeStyle3D(o.badgeStyle3D),
      layoutsInformationWindow(o.layoutsInformationWindow),
      autostart(o.autostart),
      borderlessMaximized(o.borderlessMaximized),
      contextMenuAlwaysActions(o.contextMenuAlwaysActions),
      isAvailableGeometryBroadcastedToPlasma(o.isAvailableGeometryBroadcastedToPlasma),
      metaPressForAppLauncher(o.metaPressForAppLauncher),
      metaHoldForBadges(o.metaHoldForBadges),
      mouseSensitivity(o.mouseSensitivity),
      parabolicSpread(o.parabolicSpread),
      thicknessMarginInfluence(o.thicknessMarginInfluence),
      screensDelay(o.screensDelay)
{
}

Preferences::Preferences(const Preferences &o)
    : badgeStyle3D(o.badgeStyle3D),
      layoutsInformationWindow(o.layoutsInformationWindow),
      autostart(o.autostart),
      borderlessMaximized(o.borderlessMaximized),
      contextMenuAlwaysActions(o.contextMenuAlwaysActions),
      isAvailableGeometryBroadcastedToPlasma(o.isAvailableGeometryBroadcastedToPlasma),
      metaPressForAppLauncher(o.metaPressForAppLauncher),
      metaHoldForBadges(o.metaHoldForBadges),
      mouseSensitivity(o.mouseSensitivity),
      parabolicSpread(o.parabolicSpread),
      thicknessMarginInfluence(o.thicknessMarginInfluence),
      screensDelay(o.screensDelay)
{
}

Preferences &Preferences::operator=(const Preferences &rhs)
{
    badgeStyle3D = rhs.badgeStyle3D;
    layoutsInformationWindow = rhs.layoutsInformationWindow;
    autostart = rhs.autostart;
    borderlessMaximized = rhs.borderlessMaximized;
    contextMenuAlwaysActions = rhs.contextMenuAlwaysActions;
    isAvailableGeometryBroadcastedToPlasma = rhs.isAvailableGeometryBroadcastedToPlasma;
    metaPressForAppLauncher = rhs.metaPressForAppLauncher;
    metaHoldForBadges = rhs.metaHoldForBadges;
    mouseSensitivity = rhs.mouseSensitivity;
    parabolicSpread = rhs.parabolicSpread;
    thicknessMarginInfluence = rhs.thicknessMarginInfluence;
    screensDelay = rhs.screensDelay;

    return (*this);
}

Preferences &Preferences::operator=(Preferences &&rhs)
{
    badgeStyle3D = rhs.badgeStyle3D;
    layoutsInformationWindow = rhs.layoutsInformationWindow;
    autostart = rhs.autostart;
    borderlessMaximized = rhs.borderlessMaximized;
    contextMenuAlwaysActions = rhs.contextMenuAlwaysActions;
    isAvailableGeometryBroadcastedToPlasma = rhs.isAvailableGeometryBroadcastedToPlasma;
    metaPressForAppLauncher = rhs.metaPressForAppLauncher;
    metaHoldForBadges = rhs.metaHoldForBadges;
    mouseSensitivity = rhs.mouseSensitivity;
    parabolicSpread = rhs.parabolicSpread;
    thicknessMarginInfluence = rhs.thicknessMarginInfluence;
    screensDelay = rhs.screensDelay;

    return (*this);
}

bool Preferences::operator==(const Preferences &rhs) const
{
    return (badgeStyle3D == rhs.badgeStyle3D)
            && (layoutsInformationWindow == rhs.layoutsInformationWindow)
            && (autostart == rhs.autostart)
            && (borderlessMaximized == rhs.borderlessMaximized)
            && (contextMenuAlwaysActions == rhs.contextMenuAlwaysActions)
            && (isAvailableGeometryBroadcastedToPlasma == rhs.isAvailableGeometryBroadcastedToPlasma)
            && (metaPressForAppLauncher == rhs.metaPressForAppLauncher)
            && (metaHoldForBadges == rhs.metaHoldForBadges)
            && (mouseSensitivity == rhs.mouseSensitivity)
            && (parabolicSpread == rhs.parabolicSpread)
            && (thicknessMarginInfluence == rhs.thicknessMarginInfluence)
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
            && (contextMenuAlwaysActions == Data::ContextMenu::ACTIONSALWAYSVISIBLE)
            && (isAvailableGeometryBroadcastedToPlasma == ISAVAILABLEGEOMETRYBROADCASTEDTOPLASMA)
            && (metaPressForAppLauncher == METAPRESSFORAPPLAUNCHER)
            && (metaHoldForBadges == METAHOLDFORBADGES)
            && (mouseSensitivity == MOUSESENSITIVITY)
            && (parabolicSpread == PARABOLICSPREAD)
            && (thicknessMarginInfluence == THICKNESSMARGININFLUENCE)
            && (screensDelay == SCREENSDELAY);
}

void Preferences::setToDefaults()
{
    badgeStyle3D = BADGE3DSTYLE;
    layoutsInformationWindow = LAYOUTSINFORMATIONWINDOW;
    autostart = AUTOSTART;
    borderlessMaximized = BORDERLESSMAXIMIZED;
    contextMenuAlwaysActions = Data::ContextMenu::ACTIONSALWAYSVISIBLE;
    isAvailableGeometryBroadcastedToPlasma = ISAVAILABLEGEOMETRYBROADCASTEDTOPLASMA;
    metaPressForAppLauncher = METAPRESSFORAPPLAUNCHER;
    metaHoldForBadges = METAHOLDFORBADGES;
    mouseSensitivity = MOUSESENSITIVITY;
    parabolicSpread = PARABOLICSPREAD;
    thicknessMarginInfluence = THICKNESSMARGININFLUENCE;
    screensDelay = SCREENSDELAY;
}


}
}
