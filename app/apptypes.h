/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
*
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
    SingleLayout = 0,  /* a single Layout is loaded in each time */
    MultipleLayouts  /* multiple layouts are loaded on runtime,based on Activities and one central layout for the rest unassigned Activities */
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
