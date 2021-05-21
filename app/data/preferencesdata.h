/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef PREFERENCESDATA_H
#define PREFERENCESDATA_H

// local
#include "../apptypes.h"

namespace Latte {
namespace Data {

class Preferences
{
public:
    static const bool BADGE3DSTYLE = true;
    static const bool LAYOUTSINFORMATIONWINDOW = true;
    static const bool AUTOSTART = true;
    static const bool BORDERLESSMAXIMIZED = false;
    static const bool METAPRESSFORAPPLAUNCHER = false;
    static const bool METAHOLDFORBADGES = true;
    static const int SCREENSDELAY = 2500;
    static const Settings::MouseSensitivity MOUSESENSITIVITY = Settings::HighMouseSensitivity;

    Preferences();
    Preferences(Preferences &&o);
    Preferences(const Preferences &o);

    //! Preferences data
    bool badgeStyle3D{BADGE3DSTYLE};
    bool layoutsInformationWindow{LAYOUTSINFORMATIONWINDOW};
    bool autostart{AUTOSTART};
    bool borderlessMaximized{BORDERLESSMAXIMIZED};
    bool metaPressForAppLauncher{METAPRESSFORAPPLAUNCHER};
    bool metaHoldForBadges{METAHOLDFORBADGES};
    int screensDelay{SCREENSDELAY};
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
