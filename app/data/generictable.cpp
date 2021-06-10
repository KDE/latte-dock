/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// local
#include "generictable.h"
#include "activitydata.h"
#include "appletdata.h"
#include "errorinformationdata.h"
#include "layoutdata.h"
#include "screendata.h"
#include "viewdata.h"

// Qt
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
GenericTable<T>::operator QString() const
{
    QString result;

    for(int i=0; i<m_list.count(); ++i) {
        result += m_list[i].id;
        if (i<(m_list.count()-1)) {
            result += ", ";
        }
    }

    return result;
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
bool GenericTable<T>::isEmpty() const
{
    return m_list.count() <= 0;
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
QStringList GenericTable<T>::ids() const
{
    QStringList idlist;

    for(int i=0; i<m_list.count(); ++i) {
        idlist << m_list[i].id;
    }

    return idlist;
}

template <class T>
QStringList GenericTable<T>::names() const
{
    QStringList nms;

    for(int i=0; i<m_list.count(); ++i) {
        nms << m_list[i].name;
    }

    return nms;
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
template class GenericTable<Data::Activity>;
template class GenericTable<Data::Applet>;
template class GenericTable<Data::ErrorInformation>;
template class GenericTable<Data::Generic>;
template class GenericTable<Data::Layout>;
template class GenericTable<Data::Screen>;
template class GenericTable<Data::View>;

}
}
