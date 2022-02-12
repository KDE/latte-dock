/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PREFERENCESDATA_H
#define PREFERENCESDATA_H

// local
#include "contextmenudata.h"
#include "../apptypes.h"

// Qt
#include <QStringList>

namespace Latte {
namespace Data {

class Preferences
{
public:
    static const bool BADGE3DSTYLE = true;
    static const bool LAYOUTSINFORMATIONWINDOW = true;
    static const bool AUTOSTART = true;
    static const bool BORDERLESSMAXIMIZED = false;
    static const bool ISAVAILABLEGEOMETRYBROADCASTEDTOPLASMA = true;
    static const bool METAPRESSFORAPPLAUNCHER = false;
    static const bool METAHOLDFORBADGES = true;
    static const int PARABOLICSPREAD = 3;
    static const int SCREENSDELAY = 2500;
    static const float THICKNESSMARGININFLUENCE;
    static const Settings::MouseSensitivity MOUSESENSITIVITY = Settings::HighMouseSensitivity;

    Preferences();
    Preferences(Preferences &&o);
    Preferences(const Preferences &o);

    //! Preferences data
    bool badgeStyle3D{BADGE3DSTYLE};
    bool layoutsInformationWindow{LAYOUTSINFORMATIONWINDOW};
    bool autostart{AUTOSTART};
    bool borderlessMaximized{BORDERLESSMAXIMIZED};
    bool isAvailableGeometryBroadcastedToPlasma{ISAVAILABLEGEOMETRYBROADCASTEDTOPLASMA};
    bool metaPressForAppLauncher{METAPRESSFORAPPLAUNCHER};
    bool metaHoldForBadges{METAHOLDFORBADGES};
    int parabolicSpread{PARABOLICSPREAD};
    int screensDelay{SCREENSDELAY};
    float thicknessMarginInfluence{THICKNESSMARGININFLUENCE};
    QStringList contextMenuAlwaysActions{Data::ContextMenu::ACTIONSALWAYSVISIBLE};
    Settings::MouseSensitivity mouseSensitivity{MOUSESENSITIVITY};

    bool inDefaultValues() const;
    void setToDefaults();

    //! Operators
    Preferences &operator=(const Preferences &rhs);
    Preferences &operator=(Preferences &&rhs);
    bool operator==(const Preferences &rhs) const;
    bool operator!=(const Preferences &rhs) const;
};

}
}

#endif
