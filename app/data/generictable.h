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

#ifndef GENERICTABLEDATA_H
#define GENERICTABLEDATA_H

// local
#include "genericdata.h"

// Qt
#include <QList>

namespace Latte {
namespace Data {

template <class T>
class GenericTable
{

public:
    GenericTable();
    GenericTable(GenericTable<T> &&o);
    GenericTable(const GenericTable<T> &o);

    //! Operators
    GenericTable<T> &operator=(const GenericTable<T> &rhs);
    GenericTable<T> &operator=(GenericTable<T> &&rhs);
    GenericTable<T> &operator<<(const T &rhs);
    GenericTable<T> &operator<<(const GenericTable<T> &rhs);
    GenericTable<T> &insert(const int &pos, const T &rhs);
    GenericTable<T> &insertBasedOnName(const T &rhs);
    GenericTable<T> &insertBasedOnId(const T &rhs);

    bool operator==(const GenericTable<T> &rhs) const;
    bool operator!=(const GenericTable<T> &rhs) const;
    T &operator[](const QString &id);
    const T operator[](const QString &id) const;
    T &operator[](const uint &index);
    const T operator[](const uint &index) const;
    operator QString() const;

    bool containsId(const QString &id) const;
    bool containsName(const QString &name) const;
    bool isEmpty() const;
    bool rowExists(const int &row) const;

    int indexOf(const QString &id) const;
    int rowCount() const;
    int sortedPosForName(const QString &name) const;
    int sortedPosForId(const QString &id) const;

    QString idForName(const QString &name) const;

    QStringList names() const;

    void clear();
    void remove(const int &row);
    void remove(const QString &id);

protected:
    //! #id, record
    QList<T> m_list;

};

}
}

#endif
