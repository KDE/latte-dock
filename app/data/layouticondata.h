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

#ifndef LAYOUTICONDATA_H
#define LAYOUTICONDATA_H

//! local
#include "genericdata.h"

//! Qt
#include <QMetaType>
#include <QString>

namespace Latte {
namespace Data {

class LayoutIcon : public Generic
{
public:
    LayoutIcon();
    LayoutIcon(LayoutIcon &&o);
    LayoutIcon(const LayoutIcon &o);

    //! Layout data
    bool isBackgroundFile{true};

    bool isEmpty() const;

    //! Operators
    LayoutIcon &operator=(const LayoutIcon &rhs);
    LayoutIcon &operator=(LayoutIcon &&rhs);
    bool operator==(const LayoutIcon &rhs) const;
    bool operator!=(const LayoutIcon &rhs) const;
};

}
}

Q_DECLARE_METATYPE(Latte::Data::LayoutIcon)

#endif
