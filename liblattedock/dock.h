/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DOCK_H
#define DOCK_H

#include <QObject>
#include <QMetaEnum>
#include <QMetaType>

namespace Latte {

class Dock
{
    Q_GADGET

public:
    Dock() = delete;
    ~Dock() {}

    enum Visibility
    {
        None = -1,
        AlwaysVisible = 0,
        AutoHide,
        DodgeActive,
        DodgeMaximized,
        DodgeAllWindows,
        WindowsGoBelow
    };
    Q_ENUM(Visibility)

    enum Alignment
    {
        Center = 0,
        Left,
        Right,
        Top,
        Bottom,
        Justify = 10
    };
    Q_ENUM(Alignment)

    enum SessionType
    {
        DefaultSession = 0,
        AlternativeSession
    };
    Q_ENUM(SessionType)

    enum Modifier
    {
        Shift = 0,
        Ctrl,
        Alt,
        Meta
    };
    Q_ENUM(Modifier)

    enum ClickAction
    {
        LeftClick = 0,
        MiddleClick,
        RightClick
    };
    Q_ENUM(ClickAction)

    enum TaskAction
    {
        NoneAction = 0,
        Close,
        NewInstance,
        ToggleMinimized,
        CycleThroughTasks,
        ToggleGrouping
    };
    Q_ENUM(TaskAction)

    enum ActiveAppletIndicator
    {
        NoneIndicator = 0,
        InternalsIndicator = 1,
        AllIndicator = 2
    };
    Q_ENUM(ActiveAppletIndicator)

    enum ActiveIndicatorType
    {
        LineIndicator = 0,
        DotIndicator = 1
    };
    Q_ENUM(ActiveIndicatorType)

    enum LaunchersGroup
    {
        UniqueLaunchers = 0,
        LayoutLaunchers = 1,
        GlobalLaunchers = 2
    };
    Q_ENUM(LaunchersGroup)

    enum GlowGroup
    {
        GlowOnlyOnActive = 0,
        GlowAll = 1
    };
    Q_ENUM(GlowGroup)

    enum LayoutsMemoryUsage
    {
        SingleLayout = 0,  /* a single Layout is loaded in each time */
        MultipleLayouts  /* multiple layouts are loaded on runtime,based on Activities and one central layout for the rest unassigned Activities */
    };
    Q_ENUM(LayoutsMemoryUsage)

    enum MouseSensitivity
    {
        LowSensitivity = 0,
        MediumSensitivity,
        HighSensitivity
    };
    Q_ENUM(MouseSensitivity)

    enum LatteConfigPage
    {
        LayoutPage = 0,
        PreferencesPage
    };
    Q_ENUM(LatteConfigPage)
};

}//end of namespace
#endif
