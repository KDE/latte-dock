/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewstable.h"

#include <QDebug>

namespace Latte {
namespace Data {

const char *TEMPIDPREFIX = "temp:";

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

bool ViewsTable::hasContainmentId(const QString &cid) const
{
    if (containsId(cid)) {
        return true;
    }

    for(int i=0; i<rowCount(); ++i) {
        if (m_list[i].subcontainments.containsId(cid)) {
            return true;
        }
    }

    return false;
}

ViewsTable ViewsTable::subtracted(const ViewsTable &rhs) const
{
    ViewsTable subtract;

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

ViewsTable ViewsTable::onlyOriginals() const
{
    ViewsTable originals;

    for(int i=0; i<m_list.count(); ++i) {
        if (m_list[i].isOriginal()) {
            originals << m_list[i];
        }
    }

    return originals;
}

void ViewsTable::appendTemporaryView(const Data::View &view)
{
    int maxTempId = 0;

    for(int i=0; i<rowCount(); ++i) {
        if ((*this)[i].id.startsWith(TEMPIDPREFIX)) {
            QString tid = (*this)[i].id;
            tid.remove(0, QString(TEMPIDPREFIX).count());
            if (tid.toInt() > maxTempId) {
                maxTempId = tid.toInt();
            }
        }
    }

    Data::View newview = view;
    newview.id =  QString(TEMPIDPREFIX + QString::number(maxTempId+1));
    m_list << newview;
}

void ViewsTable::print()
{
    qDebug().noquote() << "Views initialized : " + (isInitialized ? QString("true") : QString("false"));
    qDebug().noquote() << "aa | id | active | primary | screen | edge | alignment | maxlength | subcontainments";

    for(int i=0; i<rowCount(); ++i) {
        qDebug().noquote() << QString::number(i+1) << " | " << m_list[i];
    }
}

}
}
