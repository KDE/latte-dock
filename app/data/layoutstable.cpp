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

#include <QDebug>

namespace Latte {
namespace Data {

LayoutsTable::LayoutsTable()
    : GenericTable<Layout>()
{
}

LayoutsTable::LayoutsTable(LayoutsTable &&o)
    : GenericTable<Layout>(o)
{

}

LayoutsTable::LayoutsTable(const LayoutsTable &o)
    : GenericTable<Layout>(o)
{

}

//! Operators
LayoutsTable &LayoutsTable::operator=(const LayoutsTable &rhs)
{
    m_list = rhs.m_list;
    return (*this);
}

LayoutsTable &LayoutsTable::operator=(LayoutsTable &&rhs)
{
    m_list = rhs.m_list;
    return (*this);
}

LayoutsTable LayoutsTable::subtracted(const LayoutsTable &rhs) const
{
    LayoutsTable subtract;

    if ((*this) == rhs) {
        return subtract;
    }

    for(int i=0; i<m_list.count(); ++i) {
        if (!rhs.containsId(m_list[i].id)) {
            subtract << m_list[i];
        }
    }

    return subtract;
}

void LayoutsTable::setLayoutForFreeActivities(const QString &id)
{
    int row = indexOf(id);


    if (row>=0) {
        m_list[row].activities = QStringList(Data::Layout::FREEACTIVITIESID);
    }
}

}
}
