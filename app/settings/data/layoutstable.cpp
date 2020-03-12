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
        m_layouts << rhs;
    }

    return (*this);
}

bool LayoutsTable::operator==(const LayoutsTable &rhs) const
{
    if (m_layouts.count() == 0 && rhs.m_layouts.count() == 0) {
        return true;
    }

    if (m_layouts.count() != rhs.m_layouts.count()) {
        return false;
    }

    for(int i=0; i<m_layouts.count(); ++i) {
        if (m_layouts[i] != rhs.m_layouts[i]){
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
    int pos{-1};

    for(int i=0; i<m_layouts.count(); ++i) {
        if (m_layouts[i].id == id){
            pos = i;
            break;
        }
    }

    return m_layouts[pos];
}

const Layout LayoutsTable::operator[](const QString &id) const
{
    int pos{-1};

    for(int i=0; i<m_layouts.count(); ++i) {
        if (m_layouts[i].id == id){
            pos = i;
            break;
        }
    }

    return m_layouts[pos];
}

Layout &LayoutsTable::operator[](const uint &index)
{
    return m_layouts[index];
}

const Layout LayoutsTable::operator[](const uint &index) const
{
    return m_layouts[index];
}

bool LayoutsTable::contains(const QString &id) const
{
    for(int i=0; i<m_layouts.count(); ++i) {
        if (m_layouts[i].id == id){
            return true;
        }
    }

    return false;
}

bool LayoutsTable::rowExists(const int &row) const
{
    return (m_layouts.count()>=0 && row>=0 && row<rowCount());
}

int LayoutsTable::indexOf(const QString &id) const
{
    for(int i=0; i<m_layouts.count(); ++i) {
        if (m_layouts[i].id == id){
            return i;
        }
    }

    return -1;
}

int LayoutsTable::rowCount() const
{
    return m_layouts.count();
}

QString LayoutsTable::idForName(const QString &name) const
{
    for(int  i=0; i<m_layouts.count(); ++i) {
        if (m_layouts[i].name == name) {
            return m_layouts[i].id;
        }
    }

    return QString();
}

void LayoutsTable::clear()
{
    m_layouts.clear();
}

void LayoutsTable::removeLayout(const QString &id)
{
    const int pos = indexOf(id);

    if (pos >= 0) {
        m_layouts.removeAt(pos);
    }
}

void LayoutsTable::remove(const int &row)
{
    if (rowExists(row)) {
        m_layouts.removeAt(row);
    }
}


}
}
}
