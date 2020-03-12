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

#include "layoutsmodel.h"
#include "../data/layoutdata.h"

// Qt
#include <QDebug>

// KDE
#include <KLocalizedString>


const QChar CheckMark{0x2714};

namespace Latte {
namespace Settings {
namespace Model {

Layouts::Layouts(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int Layouts::rowCount() const
{
    return m_layoutsTable.rowCount();
}

int Layouts::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_layoutsTable.rowCount();
}

int Layouts::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return SHAREDCOLUMN;
}

void Layouts::clear()
{
    if (m_layoutsTable.rowCount() > 1) {
        beginInsertRows(QModelIndex(), 0, m_layoutsTable.rowCount() - 1);
        m_layoutsTable.clear();
        endInsertRows();
    }
}

void Layouts::appendLayout(const Settings::Data::Layout &layout)
{
    beginInsertRows(QModelIndex(), m_layoutsTable.rowCount(), m_layoutsTable.rowCount());
    m_layoutsTable << layout;
    endInsertRows();
}

void Layouts::removeLayout(const QString &id)
{
    if (m_layoutsTable.contains(id)) {
        beginInsertRows(QModelIndex(), m_layoutsTable.rowCount(), m_layoutsTable.rowCount());
        m_layoutsTable.removeLayout(id);
        endInsertRows();
    }
}

void Layouts::remove(const int &row)
{
    if (m_layoutsTable.rowExists(row)) {
        beginInsertRows(QModelIndex(), m_layoutsTable.rowCount(), m_layoutsTable.rowCount());
        m_layoutsTable.remove(row);
        endInsertRows();
    }
}

QVariant Layouts::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal) {
        return QVariant{};
    }

    switch(section) {
    case IDCOLUMN:
        return QString("#path");
    case HIDDENTEXTCOLUMN:
        //! QIcon::fromTheme("games-config-background")
        return QString(i18nc("column for layout background", "Background"));
    case COLORCOLUMN:
        return QString(i18nc("column for layout name", "Name"));
    case NAMECOLUMN:
        return QString(i18nc("column for layout to show in menu", "In Menu"));
    case MENUCOLUMN:
        return QString(i18nc("column for layout to show in menu", "In Menu"));
    case BORDERSCOLUMN:
        return QString(i18nc("column for layout to hide borders for maximized windows", "Borderless"));
    case ACTIVITYCOLUMN:
        //! QIcon::fromTheme("activities")
        return QString(i18nc("column for layout to show which activities is assigned to", "Activities"));
    case SHAREDCOLUMN:
        //! QIcon::fromTheme("document-share")
        return QString(i18nc("column for shared layout to show which layouts is assigned to", "Shared To"));
    default:
        break;
    };

    return QVariant{};
}

QVariant Layouts::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    int column = index.column();

    if (!m_layoutsTable.rowExists(row)) {
        return QVariant{};
    }

    switch (column) {
    case IDCOLUMN:
        if (role == Qt::DisplayRole) {
            return m_layoutsTable[row].id;
        }
        break;
    case HIDDENTEXTCOLUMN:
        return QVariant{};
    case COLORCOLUMN:
        return m_layoutsTable[row].background;
        break;
    case NAMECOLUMN:
        if (role == Qt::DisplayRole) {
            return m_layoutsTable[row].name;
        }
        break;
    case MENUCOLUMN:
        if (role == Qt::DisplayRole) {
            return m_layoutsTable[row].isShownInMenu ? CheckMark : QVariant{};
        }
        break;
    case BORDERSCOLUMN:
        if (role == Qt::DisplayRole) {
            return m_layoutsTable[row].hasDisabledBorders ? CheckMark : QVariant{};
        }
        break;
    case ACTIVITYCOLUMN:
        if (role == Qt::DisplayRole || role == Qt::UserRole) {
            return m_layoutsTable[row].activities;
        }
        break;
    case SHAREDCOLUMN:
        if (role == Qt::DisplayRole || role == Qt::UserRole) {
            return m_layoutsTable[row].shares;
        }
    default:
        return QVariant{};
    };

    return QVariant{};
}

const Data::Layout &Layouts::at(const int &row)
{
    return m_layoutsTable[row];
}

const Data::LayoutsTable &Layouts::currentData()
{
    return m_layoutsTable;
}


}
}
}

