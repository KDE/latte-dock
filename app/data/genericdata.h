/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GENERICDATA_H
#define GENERICDATA_H

// local
#include "generictable.h"

// Qt
#include <QString>

namespace Latte {
namespace Data {

class Generic
{
public:
    Generic() = default;
    Generic(Generic &&o);
    Generic(const Generic &o);

    //! Layout data
    QString id;
    QString name;

    //! Operators
    Generic &operator=(const Generic &rhs);
    Generic &operator=(Generic &&rhs);
    bool operator==(const Generic &rhs) const;
    bool operator!=(const Generic &rhs) const;
};

}
}



#endif
