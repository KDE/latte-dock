/*
 *  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef LATTETASKSTYPES_H
#define LATTETASKSTYPES_H

// Qt
#include <QObject>
#include <QMetaEnum>
#include <QMetaType>

namespace Latte {
namespace Tasks {

class Types
{
    Q_GADGET

public:
    Types() = delete;
    ~Types() {}

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

    enum TaskScrollAction
    {
        ScrollNone = 0,
        ScrollTasks,
        ScrollToggleMinimized
    };
    Q_ENUM(TaskScrollAction);

    enum ManualScrollType
    {
        ManualScrollDisabled = 0,
        ManualScrollOnlyParallel,
        ManualScrollVerticalHorizontal
    };
    Q_ENUM(ManualScrollType);
};

}
}

#endif
