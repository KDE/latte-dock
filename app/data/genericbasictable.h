/*
 * Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef GENERICBASICTABLE_H
#define GENERICBASICTABLE_H

// local
#include "genericdata.h"
#include "generictable.h"

// Qt
#include <QMetaType>

namespace Latte {
namespace Data {

class GenericBasicTable : public GenericTable<Generic>
{
public:
    GenericBasicTable();
    GenericBasicTable(GenericBasicTable &&o);
    GenericBasicTable(const GenericBasicTable &o);
};

}
}

Q_DECLARE_METATYPE(Latte::Data::Generic)
Q_DECLARE_METATYPE(Latte::Data::GenericBasicTable)

#endif
