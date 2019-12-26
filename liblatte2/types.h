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

#ifndef TYPES_H
#define TYPES_H

// Qt
#include <QObject>
#include <QMetaEnum>
#include <QMetaType>

namespace Latte {

class Types
{
    Q_GADGET

public:
    Types() = delete;
    ~Types() {}

    enum ViewType
    {
        DockView = 0,
        PanelView
    };
    Q_ENUM(ViewType);

    enum Visibility
    {
        None = -1,
        AlwaysVisible = 0,
        AutoHide,
        DodgeActive,
        DodgeMaximized,
        DodgeAllWindows,
        WindowsGoBelow,
        WindowsCanCover,
        WindowsAlwaysCover
    };
    Q_ENUM(Visibility);

    enum Alignment
    {
        Center = 0,
        Left,
        Right,
        Top,
        Bottom,
        Justify = 10
    };
    Q_ENUM(Alignment);

    enum EdgesAndAlignments
    {
        BottomEdgeCenterAlign = 0,
        BottomEdgeLeftAlign,
        BottomEdgeRightAlign,
        TopEdgeCenterAlign,
        TopEdgeLeftAlign,
        TopEdgeRightAlign,
        LeftEdgeCenterAlign,
        LeftEdgeTopAlign,
        LeftEdgeBottomAlign,
        RightEdgeCenterAlign,
        RightEdgeTopAlign,
        RightEdgeBottomAlign
    };
    Q_ENUM(EdgesAndAlignments);

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
    Q_ENUM(Modifier);

    enum ClickAction
    {
        LeftClick = 0,
        MiddleClick,
        RightClick
    };
    Q_ENUM(ClickAction);

    enum ManualScrollType
    {
        ManualScrollDisabled = 0,
        ManualScrollOnlyParallel,
        ManualScrollVerticalHorizontal
    };
    Q_ENUM(ManualScrollType);

    enum ScrollAction
    {
        ScrollNone = 0,
        ScrollDesktops,
        ScrollActivities,
        ScrollTasks
    };
    Q_ENUM(ScrollAction);

    enum TaskAction
    {
        NoneAction = 0,
        Close,
        NewInstance,
        ToggleMinimized,
        CycleThroughTasks,
        ToggleGrouping,
        PresentWindows,
        PreviewWindows,
        HighlightWindows,
        PreviewAndHighlightWindows
    };
    Q_ENUM(TaskAction);

    enum IndicatorStyle
    {
        LatteIndicator = 0,
        PlasmaIndicator,
        UnityIndicator
    };
    Q_ENUM(IndicatorStyle);

    enum ActiveIndicatorType
    {
        LineIndicator = 0,
        DotIndicator = 1
    };
    Q_ENUM(ActiveIndicatorType);

    enum LaunchersGroup
    {
        UniqueLaunchers = 0,
        LayoutLaunchers = 1,
        GlobalLaunchers = 2
    };
    Q_ENUM(LaunchersGroup);

    enum GlowGroup
    {
        GlowNone = -1,
        GlowOnlyOnActive = 0,
        GlowAll = 1
    };
    Q_ENUM(GlowGroup);

    enum ShadowGroup
    {
        NoneAppletShadow = 0,
        LockedAppletsShadow, /* DEPRECATED, apply shadow only to locked applets */
        AllAppletsShadow
    };
    Q_ENUM(ShadowGroup);

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
        SmartThemeColors
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

    enum LayoutsMemoryUsage
    {
        SingleLayout = 0,  /* a single Layout is loaded in each time */
        MultipleLayouts  /* multiple layouts are loaded on runtime,based on Activities and one central layout for the rest unassigned Activities */
    };
    Q_ENUM(LayoutsMemoryUsage);

    enum MouseSensitivity
    {
        LowSensitivity = 0,
        MediumSensitivity,
        HighSensitivity
    };
    Q_ENUM(MouseSensitivity);

    enum LatteConfigPage
    {
        LayoutPage = 0,
        PreferencesPage
    };
    Q_ENUM(LatteConfigPage);

    enum SettingsComplexity
    {
        BasicSettings = 0,
        AdvancedSettings = 2,
        ExpertSettings = 4
    };
    Q_ENUM(SettingsComplexity);

    enum ImportExportState
    {
        Failed = 0,
        Updated = 2,
        Installed = 4
    };
    Q_ENUM(ImportExportState);
};

}//end of namespace
#endif
