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

#ifndef SETTINGSDATALAYOUTSTABLE_H
#define SETTINGSDATALAYOUTSTABLE_H

#include "layout.h"

#include <QHash>

namespace Latte {
namespace Settings {
namespace Data {

class LayoutsTable
{

public:
    LayoutsTable();
    ~LayoutsTable();

    //! Operators
    LayoutsTable &operator=(const LayoutsTable &rhs);
    LayoutsTable &operator<<(const Layout &rhs);
    bool operator==(const LayoutsTable &rhs) const;
    bool operator!=(const LayoutsTable &rhs) const;
    Layout &operator[](const QString &id);
    const Layout &operator[](const QString &id) const;

    bool contains(const QString &id) const;

protected:
    //! #id, layout_record
    QHash<QString, Layout> m_layouts;

};

}
}
}

#endif
