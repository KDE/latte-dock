/*
 * Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "appletsmodel.h"

// local
#include "../../layout/abstractlayout.h"

// KDE
#include <KLocalizedString>

namespace Latte {
namespace Settings {
namespace Model {

Applets::Applets(QObject *parent, Latte::Corona *corona)
    : QAbstractTableModel(parent),
      m_corona(corona)
{
}

Applets::~Applets()
{
}

int Applets::rowCount() const
{
    return m_appletsTable.rowCount();
}

int Applets::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_appletsTable.rowCount();
}

int Applets::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 1;
}

int Applets::row(const QString &id)
{
    for (int i=0; i<m_appletsTable.rowCount(); ++i){
        if (m_appletsTable[i].id == id) {
            return i;
        }
    }

    return -1;
}

void Applets::clear()
{
    if (m_appletsTable.rowCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_appletsTable.rowCount() - 1);
        m_appletsTable.clear();
        endRemoveRows();
    }
}

void Applets::setData(const Latte::Data::AppletsTable &applets)
{
    clear();

    if (applets.rowCount() > 0) {
        beginInsertRows(QModelIndex(), 0, applets.rowCount()-1);
        m_appletsTable = applets;
        endInsertRows();
    }
}

QVariant Applets::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    if (role == Qt::FontRole) {
        QFont font = qvariant_cast<QFont>(QAbstractTableModel::headerData(section, orientation, role));
        font.setBold(true);
        return font;
    }

    switch(section) {
    case NAMECOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("column for current applets", "Current Applets"));
        }
        break;
    default:
        break;
    };

    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant Applets::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    int column = index.column();

    if (row >= rowCount()) {
        return QVariant{};
    }

    if (role == NAMEROLE || role == Qt::DisplayRole) {
        return m_appletsTable[row].name;
    } else if (role == IDROLE) {
            return m_appletsTable[row].id;
    } else if (role == SELECTEDROLE) {
        return m_appletsTable[row].isSelected;
    } else if (role == ICONROLE) {
        return m_appletsTable[row].icon;
    } else if (role == DESCRIPTIONROLE) {
        return m_appletsTable[row].description;
    } else if (role == SORTINGROLE) {
        return m_appletsTable[row].isInstalled() ? QString(1000 + m_appletsTable[row].name) : m_appletsTable[row].name;
    }

    return QVariant{};
}

}
}
}
