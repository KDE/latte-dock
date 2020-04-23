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

#endif
