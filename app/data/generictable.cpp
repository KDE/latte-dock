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

#include "generictable.h"
#include "appletdata.h"
#include "layoutdata.h"

#include <QDebug>

namespace Latte {
namespace Data {

template <class T>
GenericTable<T>::GenericTable()
{
}

template <class T>
GenericTable<T>::GenericTable(GenericTable<T> &&o)
    : m_list(o.m_list)
{

}

template <class T>
GenericTable<T>::GenericTable(const GenericTable<T> &o)
    : m_list(o.m_list)
{

}

//! Operators
template <class T>
GenericTable<T> &GenericTable<T>::operator=(const GenericTable<T> &rhs)
{
    m_list = rhs.m_list;

    return (*this);
}

template <class T>
GenericTable<T> &GenericTable<T>::operator=(GenericTable<T> &&rhs)
{
    m_list = rhs.m_list;
    return (*this);
}

template <class T>
GenericTable<T> &GenericTable<T>::operator<<(const T &rhs)
{
    if (!rhs.id.isEmpty()) {
        m_list << rhs;
    }

    return (*this);
}

template <class T>
GenericTable<T> &GenericTable<T>::operator<<(const GenericTable<T> &rhs)
{
    m_list << rhs.m_list;
    return (*this);
}

template <class T>
GenericTable<T> &GenericTable<T>::insert(const int &pos, const T &rhs)
{
    m_list.insert(pos, rhs);
    return (*this);
}

template <class T>
GenericTable<T> &GenericTable<T>::insertBasedOnName(const T &rhs)
{
    return insert(sortedPosForName(rhs.name), rhs);
}

template <class T>
GenericTable<T> &GenericTable<T>::insertBasedOnId(const T &rhs)
{
    return insert(sortedPosForId(rhs.id), rhs);
}

template <class T>
bool GenericTable<T>::operator==(const GenericTable<T> &rhs) const
{
    if (m_list.count() == 0 && rhs.m_list.count() == 0) {
        return true;
    }

    if (m_list.count() != rhs.m_list.count()) {
        return false;
    }

    for(int i=0; i<m_list.count(); ++i) {
        QString id = m_list[i].id;

        if (!rhs.containsId(id) || (*this)[id] != rhs[id]){
            return false;
        }
    }

    return true;
}

template <class T>
bool GenericTable<T>::operator!=(const GenericTable<T> &rhs) const
{
    return !(*this == rhs);
}

template <class T>
T &GenericTable<T>::operator[](const QString &id)
{
    int pos{-1};

    for(int i=0; i<m_list.count(); ++i) {
        if (m_list[i].id == id){
            pos = i;
            break;
        }
    }

    return m_list[pos];
}

template <class T>
const T GenericTable<T>::operator[](const QString &id) const
{
    int pos{-1};

    for(int i=0; i<m_list.count(); ++i) {
        if (m_list[i].id == id){
            pos = i;
            break;
        }
    }

    return m_list[pos];
}

template <class T>
T &GenericTable<T>::operator[](const uint &index)
{
    return m_list[index];
}

template <class T>
const T GenericTable<T>::operator[](const uint &index) const
{
    return m_list[index];
}

template <class T>
bool GenericTable<T>::containsId(const QString &id) const
{
    for(int i=0; i<m_list.count(); ++i) {
        if (m_list[i].id == id){
            return true;
        }
    }

    return false;
}

template <class T>
bool GenericTable<T>::containsName(const QString &name) const
{
    for(int i=0; i<m_list.count(); ++i) {
        if (m_list[i].name == name){
            return true;
        }
    }

    return false;
}

template <class T>
bool GenericTable<T>::rowExists(const int &row) const
{
    return (m_list.count()>=0 && row>=0 && row<rowCount());
}

template <class T>
int GenericTable<T>::indexOf(const QString &id) const
{
    for(int i=0; i<m_list.count(); ++i) {
        if (m_list[i].id == id){
            return i;
        }
    }

    return -1;
}

template <class T>
int GenericTable<T>::rowCount() const
{
    return m_list.count();
}

template <class T>
int GenericTable<T>::sortedPosForId(const QString &id) const
{
    int pos{0};

    for(int i=0; i<m_list.count(); ++i) {
        if (QString::compare(m_list[i].id, id, Qt::CaseInsensitive) <= 0) {
            pos++;
        } else {
            break;
        }
    }

    return pos;
}

template <class T>
int GenericTable<T>::sortedPosForName(const QString &name) const
{
    int pos{0};

    for(int i=0; i<m_list.count(); ++i) {
        if (QString::compare(m_list[i].name, name, Qt::CaseInsensitive) <= 0) {
            pos++;
        } else {
            break;
        }
    }

    return pos;
}

template <class T>
QString GenericTable<T>::idForName(const QString &name) const
{
    for(int  i=0; i<m_list.count(); ++i) {
        if (m_list[i].name == name) {
            return m_list[i].id;
        }
    }

    return QString();
}

template <class T>
void GenericTable<T>::clear()
{
    m_list.clear();
}

template <class T>
void GenericTable<T>::remove(const QString &id)
{
    const int pos = indexOf(id);

    if (pos >= 0) {
        m_list.removeAt(pos);
    }
}

template <class T>
void GenericTable<T>::remove(const int &row)
{
    if (rowExists(row)) {
        m_list.removeAt(row);
    }
}

//! Make linker happy and provide which table instances will be used.
//! The alternative would be to move functions definitions in the header file
//! but that would drop readability
template class GenericTable<Data::Applet>;
template class GenericTable<Data::Layout>;

}
}
