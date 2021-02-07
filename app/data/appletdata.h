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

#ifndef APPLETDATA_H
#define APPLETDATA_H

//! local
#include "genericdata.h"
#include "generictable.h"

//! Qt
#include <QMetaType>
#include <QString>

namespace Latte {
namespace Data {

class Applet : public Generic
{
public:
    Applet();
    Applet(Applet &&o);
    Applet(const Applet &o);

    //! Layout data
    bool isSelected;
    QString description;
    QString icon;

    //! Operators
    Applet &operator=(const Applet &rhs);
    Applet &operator=(Applet &&rhs);

    bool isInstalled() const;
    bool isValid() const;
};

typedef GenericTable<Applet> AppletsTable;

}
}

Q_DECLARE_METATYPE(Latte::Data::Applet)

#endif
