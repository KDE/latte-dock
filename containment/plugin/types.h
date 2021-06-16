/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LATTECONTAINMENTTYPES_H
#define LATTECONTAINMENTTYPES_H

// Qt
#include <QObject>
#include <QMetaEnum>
#include <QMetaType>

namespace Latte {
namespace Containment {

class Types
{
    Q_GADGET

public:
    Types() = delete;
    ~Types() {}

    enum ScrollAction
    {
        ScrollNone = 0,
        ScrollDesktops,
        ScrollActivities,
        ScrollTasks,
        ScrollToggleMinimized
    };
    Q_ENUM(ScrollAction);

    enum ShadowColorGroup
    {
        DefaultColorShadow = 0,
        ThemeColorShadow,
        UserColorShadow
    };
    Q_ENUM(ShadowColorGroup);

    enum ThemeColorsGroup
    {
        PlasmaThemeColors = 0,
        ReverseThemeColors,
        SmartThemeColors,
        DarkThemeColors,
        LightThemeColors,
        LayoutThemeColors
    };
    Q_ENUM(ThemeColorsGroup);

    enum WindowColorsGroup
    {
        NoneWindowColors = 0,
        ActiveWindowColors,
        TouchingWindowColors
    };
    Q_ENUM(WindowColorsGroup);

    enum ActiveWindowFilterGroup
    {
        ActiveInCurrentScreen = 0,
        ActiveFromAllScreens
    };
    Q_ENUM(ActiveWindowFilterGroup);
};

}
}

#endif
