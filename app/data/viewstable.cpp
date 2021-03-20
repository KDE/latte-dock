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

#include "viewstable.h"

#include <QDebug>

namespace Latte {
namespace Data {

ViewsTable::ViewsTable()
    : GenericTable<View>()
{
}

ViewsTable::ViewsTable(ViewsTable &&o)
    : GenericTable<View>(o),
      isInitialized(o.isInitialized)
{

}

ViewsTable::ViewsTable(const ViewsTable &o)
    : GenericTable<View>(o),
      isInitialized(o.isInitialized)
{
}

//! Operators
ViewsTable &ViewsTable::operator=(const ViewsTable &rhs)
{
    m_list = rhs.m_list;
    isInitialized = rhs.isInitialized;
    return (*this);
}

ViewsTable &ViewsTable::operator=(ViewsTable &&rhs)
{
    m_list = rhs.m_list;
    isInitialized = rhs.isInitialized;
    return (*this);
}

bool ViewsTable::operator==(const ViewsTable &rhs) const
{
    GenericTable<View> tempView = (*this);

    return (isInitialized == rhs.isInitialized)
            && (((GenericTable<View>)*this) == ((GenericTable<View>)rhs));
}

bool ViewsTable::operator!=(const ViewsTable &rhs) const
{
    return !(*this == rhs);
}


}
}
