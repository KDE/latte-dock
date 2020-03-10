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

#include "layoutstable.h"


namespace Latte {
namespace Settings {
namespace Data {

LayoutsTable::LayoutsTable()
{
}

LayoutsTable::~LayoutsTable()
{
}

//! Operators
LayoutsTable &LayoutsTable::operator=(const LayoutsTable &rhs)
{
    m_layouts = rhs.m_layouts;

    return (*this);
}

LayoutsTable &LayoutsTable::operator<<(const Layout &rhs)
{
    if (!rhs.id.isEmpty()) {
        m_layouts[rhs.id] = rhs;
    }

    return (*this);
}

bool LayoutsTable::operator==(const LayoutsTable &rhs) const
{
    if (m_layouts.keys().count() != rhs.m_layouts.keys().count()) {
        return false;
    }

    for(const QString &id : m_layouts.keys()) {
        if (!rhs.m_layouts.contains(id) || (m_layouts[id] != rhs.m_layouts[id])) {
            return false;
        }
    }

    return true;
}

bool LayoutsTable::operator!=(const LayoutsTable &rhs) const
{
    return !(*this == rhs);
}

Layout &LayoutsTable::operator[](const QString &id)
{
    Layout tmp;

    if (m_layouts.contains(id)) {
        return m_layouts[id];
    }

    return tmp;
}

const Layout &LayoutsTable::operator[](const QString &id) const
{
    Layout tmp;

    if (m_layouts.contains(id)) {
        return m_layouts[id];
    }

    return tmp;
}

bool LayoutsTable::contains(const QString &id) const
{
    return m_layouts.contains(id);
}


}
}
}
