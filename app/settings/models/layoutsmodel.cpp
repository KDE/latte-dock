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
#include <QFont>
#include <QIcon>

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

bool Layouts::inMultipleMode() const
{
    return m_inMultipleMode;
}

void Layouts::setInMultipleMode(bool inMultiple)
{
    if (m_inMultipleMode == inMultiple) {
        return;
    }

    m_inMultipleMode = inMultiple;
}

QString Layouts::idForOriginalName(const QString &name)
{
    return m_layoutsTable.idForOriginalName(name);
}

QString Layouts::idForEditedName(const QString &name)
{
    return m_layoutsTable.idForEditedName(name);
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

    return SHAREDCOLUMN+1;
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
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    if (role == Qt::FontRole) {
        QFont font = qvariant_cast<QFont>(QAbstractTableModel::headerData(section, orientation, role));
        font.setBold(true);
        return font;
    }

    switch(section) {
    case IDCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString("#path");
        }
        break;
    case HIDDENTEXTCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString("#hidden_text");
        }
        break;
    case BACKGROUNDCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("column for layout background", "Background"));
        } else if (role == Qt::DecorationRole) {
            return QIcon::fromTheme("games-config-background");
        }
        break;
    case NAMECOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("column for layout name", "Name"));
        }
        break;
    case MENUCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("column for layout to show in menu", "In Menu"));
        }
        break;
    case BORDERSCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("column for layout to hide borders for maximized windows", "Borderless"));
        }
        break;
    case ACTIVITYCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("column for layout to show which activities is assigned to", "Activities"));
        } else if (role == Qt::DecorationRole) {
            return QIcon::fromTheme("activities");
        }
        break;
    case SHAREDCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("column for shared layout to show which layouts is assigned to", "Shared To"));
        } else if (role == Qt::DecorationRole) {
            return QIcon::fromTheme("document-share");
        }
        break;
    default:
        break;
    };

    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags Layouts::flags(const QModelIndex &index) const
{
    const int column = index.column();
    const int row = index.column();

    auto flags = QAbstractTableModel::flags(index);

    bool isShared = m_inMultipleMode && m_layoutsTable[row].isShared();

    if (column == MENUCOLUMN || column == BORDERSCOLUMN) {
        if (isShared) {
            flags &= ~Qt::ItemIsEnabled;
        } else {
            flags |= Qt::ItemIsUserCheckable;
        }
    }

    if (column == ACTIVITYCOLUMN) {
        if (isShared) {
            flags &= ~Qt::ItemIsEnabled;
        } else {
            flags |= Qt::ItemIsEditable;
        }
    }

    if (column == BACKGROUNDCOLUMN
            || column == NAMECOLUMN
            || column == SHAREDCOLUMN) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

QVariant Layouts::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    int column = index.column();

    if (!m_layoutsTable.rowExists(row)) {
        return QVariant{};
    }

    if (role == LAYOUTISACTIVEROLE) {
        return m_layoutsTable[row].isActive;
    } else if (role == LAYOUTISLOCKEDROLE) {
        return m_layoutsTable[row].isLocked;
    } else if (role == LAYOUTISSHAREDROLE) {
        return m_layoutsTable[row].isShared();
    } else if (role == INMULTIPLELAYOUTSMODE) {
        return inMultipleMode();
    }

    switch (column) {
    case IDCOLUMN:
        if (role == Qt::DisplayRole) {
            return m_layoutsTable[row].id;
        }
        break;
    case HIDDENTEXTCOLUMN:
        return QVariant{};
    case BACKGROUNDCOLUMN:
        return m_layoutsTable[row].background.isEmpty() ? m_layoutsTable[row].color : m_layoutsTable[row].background;
        break;
    case NAMECOLUMN:
        if (role == Qt::DisplayRole) {
            return m_layoutsTable[row].editedName();
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
        break;
    default:
        return QVariant{};
    };

    return QVariant{};
}

bool Layouts::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const int row = index.row();
    const int column = index.column();

    if (!m_layoutsTable.rowExists(row) || column<0 || column >= SHAREDCOLUMN) {
        return false;
    }

    QVector<int> roles;
    roles << role;

    //! common roles for all row cells
    if (role == LAYOUTISLOCKEDROLE) {
        m_layoutsTable[row].isLocked = value.toBool();
        emit dataChanged(this->index(row,0), this->index(row,SHAREDCOLUMN), roles);
        return true;
    }

    //! specific roles to each independent cell
    switch (column) {
    case IDCOLUMN:
        if (role == Qt::DisplayRole) {
            m_layoutsTable[row].id = value.toString();
            emit dataChanged(index, index, roles);
            return true;
        }
        break;
    case HIDDENTEXTCOLUMN:
        return true;
        break;
    case BACKGROUNDCOLUMN:
        if (role == Qt::BackgroundRole) {
            QString back = value.toString();

            if (back.startsWith("/")) {
                m_layoutsTable[row].background = back;
            } else {
                m_layoutsTable[row].background = QString();
                m_layoutsTable[row].color = back;
            }
            emit dataChanged(index, index, roles);
            return true;
        }
        break;
    case NAMECOLUMN:
        if (role == Qt::DisplayRole) {
            m_layoutsTable[row].setEditedName(value.toString());
            emit dataChanged(index, index, roles);
            return true;
        }
        break;
    case MENUCOLUMN:
        if (role == Qt::DisplayRole) {
            m_layoutsTable[row].isShownInMenu = value.toBool();
            emit dataChanged(index, index, roles);
            return true;
        }
        break;
    case BORDERSCOLUMN:
        if (role == Qt::DisplayRole) {
            m_layoutsTable[row].hasDisabledBorders = value.toBool();
            emit dataChanged(index, index, roles);
            return true;
        }
        break;
    case ACTIVITYCOLUMN:
        if (role == Qt::UserRole) {
            m_layoutsTable[row].activities = value.toStringList();
            emit dataChanged(index, index, roles);
            return true;
        }
        break;
    case SHAREDCOLUMN:
        if (role == Qt::UserRole) {
            m_layoutsTable[row].shares = value.toStringList();
            emit dataChanged(index, index, roles);
            return true;
        }
        break;
    };

    return false;
}

const Data::Layout &Layouts::at(const int &row)
{
    return m_layoutsTable[row];
}

const Data::LayoutsTable &Layouts::currentData()
{
    return m_layoutsTable;
}

void Layouts::setCurrentData(Data::LayoutsTable &data)
{
    clear();

    beginInsertRows(QModelIndex(), 0, data.rowCount() - 1);
    m_layoutsTable = data;
    endInsertRows();
}

}
}
}
