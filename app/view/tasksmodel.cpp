/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tasksmodel.h"

// Qt
#include <QDebug>

// Plasma
#include <PlasmaQuick/AppletQuickItem>

namespace Latte {
namespace ViewPart {

TasksModel::TasksModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int TasksModel::count() const
{
    return m_tasks.count();
}

int TasksModel::rowCount(const QModelIndex &parent) const
{
    return m_tasks.count();
}

QVariant TasksModel::data(const QModelIndex &index, int role) const
{
    bool rowIsValid = (index.row()>=0 && index.row()<m_tasks.count());
    if (!rowIsValid) {
        return QVariant();
    }

    if (role == Qt::UserRole) {
        return QVariant::fromValue(m_tasks[index.row()]);
    }

    return QVariant();
}


QHash<int, QByteArray> TasksModel::roleNames() const{
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole] = "tasks";
    return roles;
}

void TasksModel::addTask(PlasmaQuick::AppletQuickItem *plasmoid)
{
    if (plasmoid && m_tasks.contains(plasmoid)) {
        return;
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_tasks << plasmoid;
    endInsertRows();

    connect(plasmoid, &QObject::destroyed, this, [&, plasmoid](){
        removeTask(plasmoid);
    });

    emit countChanged();
}

void TasksModel::removeTask(PlasmaQuick::AppletQuickItem *plasmoid)
{
    if (!plasmoid || (plasmoid && !m_tasks.contains(plasmoid))) {
        return;
    }

    int iex = m_tasks.indexOf(plasmoid);

    beginRemoveRows(QModelIndex(), iex, iex);
    m_tasks.removeAll(plasmoid);
    endRemoveRows();

    emit countChanged();
}

}
}
