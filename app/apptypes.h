/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LATTEAPPLOCALTYPES_H
#define LATTEAPPLOCALTYPES_H

// Qt
#include <QObject>

//! These are LatteApp::Types that will be used ONLY from Latte App c++ implementation.
//! Such types are irrelevant and not used from plasma applets.

namespace Latte {
namespace ImportExport {
Q_NAMESPACE

enum State
{
    FailedState = 0,
    UpdatedState = 2,
    InstalledState = 4
};
Q_ENUM_NS(State);

}
}

namespace Latte {
namespace MemoryUsage {

enum LayoutsMemory
{
    Current = -1, /*current layouts memory usage*/
    SingleLayout = 0,  /* a single Layout is loaded in each time */
    MultipleLayouts  /* multiple layouts are loaded on runtime,based on Activities and one central layout for the rest unassigned Activities */
};

}
}

namespace Latte {
namespace MultipleLayouts {

enum Status
{
    Uninitialized = -1,
    Paused,
    Running
};

}
}

//! These are LatteApp::Types that will be used from Latte App c++ implementation AND
//! Latte containment qml. Such types are irrelevant and not used from plasma applets.

namespace Latte {
namespace Settings {
Q_NAMESPACE

enum MouseSensitivity
{
    LowMouseSensitivity = 0,
    MediumMouseSensitivity,
    HighMouseSensitivity
};
Q_ENUM_NS(MouseSensitivity);

}
}

#endif
