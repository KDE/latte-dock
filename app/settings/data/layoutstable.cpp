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

LayoutsTable::LayoutsTable(LayoutsTable &&o)
    : m_layouts(o.m_layouts)
{

}

LayoutsTable::LayoutsTable(const LayoutsTable &o)
    : m_layouts(o.m_layouts)
{

}

//! Operators
LayoutsTable &LayoutsTable::operator=(const LayoutsTable &rhs)
{
    m_layouts = rhs.m_layouts;

    return (*this);
}

LayoutsTable &LayoutsTable::operator=(LayoutsTable &&rhs)
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
        QString id = m_layouts[i].id;

        if (!rhs.containsId(id) || (*this)[id] != rhs[id]){
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

QStringList LayoutsTable::allSharesIds() const
{
    QStringList sharesIds;

    for(int i=0; i<m_layouts.count(); ++i) {
        if (m_layouts[i].isShared()) {
            for(int j=0; j<m_layouts[i].shares.count(); ++j) {
                sharesIds << m_layouts[i].shares[j];
            }
        }
    }

    return sharesIds;
}

QStringList LayoutsTable::allSharesNames() const
{
    QStringList sharesNames;

    for(int i=0; i<m_layouts.count(); ++i) {
        if (m_layouts[i].isShared()) {
            for(int j=0; j<m_layouts[i].shares.count(); ++j) {
                QString shareId = m_layouts[i].shares[j];
                int sid = indexOf(shareId);
                sharesNames << m_layouts[sid].currentName();
            }
        }
    }

    return sharesNames;
}

LayoutsTable LayoutsTable::subtracted(const LayoutsTable &rhs) const
{
    LayoutsTable subtract;

    if ((*this) == rhs) {
        return subtract;
    }

    for(int i=0; i<m_layouts.count(); ++i) {
        if (!rhs.containsId(m_layouts[i].id)) {
            subtract << m_layouts[i];
        }
    }

    return subtract;
}

Latte::Layouts::SharesMap LayoutsTable::sharesMap() const
{
    Latte::Layouts::SharesMap map;

    for(int i=0; i<m_layouts.count(); ++i) {
        if (m_layouts[i].isShared()) {
            QStringList sharesNames;

            for(int j=0; j<m_layouts[i].shares.count(); ++j) {
                QString shareId = m_layouts[i].shares[j];
                int sid = indexOf(shareId);
                sharesNames << m_layouts[sid].currentName();
            }

            map[m_layouts[i].currentName()] = sharesNames;
        }
    }

    return map;
}

bool LayoutsTable::containsId(const QString &id) const
{
    for(int i=0; i<m_layouts.count(); ++i) {
        if (m_layouts[i].id == id){
            return true;
        }
    }

    return false;
}

bool LayoutsTable::containsCurrentName(const QString &name) const
{
    for(int i=0; i<m_layouts.count(); ++i) {
        if (m_layouts[i].currentName() == name){
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

QString LayoutsTable::idForOriginalName(const QString &name) const
{
    for(int  i=0; i<m_layouts.count(); ++i) {
        if (m_layouts[i].originalName() == name) {
            return m_layouts[i].id;
        }
    }

    return QString();
}

QString LayoutsTable::idForCurrentName(const QString &name) const
{
    for(int  i=0; i<m_layouts.count(); ++i) {
        if ((m_layouts[i].currentName() == name)) {
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

void LayoutsTable::setLayoutForFreeActivities(const QString &id)
{
    int row = indexOf(id);

    if (row>=0) {
        for(int i=0; i<rowCount(); ++i) {
            if (i == row) {
                m_layouts[row].activities = QStringList(Data::Layout::FREEACTIVITIESID);
            } else if (m_layouts[i].activities.contains(Data::Layout::FREEACTIVITIESID)) {
                m_layouts[i].activities.removeAll(Data::Layout::FREEACTIVITIESID);
            }
        }
    }
}

}
}
}
