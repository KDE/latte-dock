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

// local
#include "../data/layoutdata.h"
#include "../../layouts/manager.h"
#include "../../layouts/synchronizer.h"

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

Layouts::Layouts(QObject *parent, Latte::Corona *corona)
    : QAbstractTableModel(parent),
      m_corona(corona)
{
    connect(this, &Layouts::inMultipleModeChanged, this, [&]() {
        QVector<int> roles;
        roles << Qt::DisplayRole;
        roles << Qt::UserRole;
        roles << INMULTIPLELAYOUTSROLE;

        emit dataChanged(index(0, MENUCOLUMN), index(rowCount(), SHAREDCOLUMN), roles);
    });

    connect(m_corona->layoutsManager(), &Latte::Layouts::Manager::currentLayoutNameChanged, this, &Layouts::updateActiveStates);
    connect(m_corona->layoutsManager(), &Latte::Layouts::Manager::centralLayoutsChanged, this, &Layouts::updateActiveStates);
}

bool Layouts::containsCurrentName(const QString &name) const
{
    return m_layoutsTable.containsCurrentName(name);
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
    emit inMultipleModeChanged();
}

QString Layouts::idForOriginalName(const QString &name)
{
    return m_layoutsTable.idForOriginalName(name);
}

QString Layouts::idForCurrentName(const QString &name)
{
    return m_layoutsTable.idForCurrentName(name);
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
    if (m_layoutsTable.rowCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_layoutsTable.rowCount() - 1);
        m_layoutsTable.clear();
        endRemoveRows();
    }
}

void Layouts::appendLayout(const Settings::Data::Layout &layout)
{
    beginInsertRows(QModelIndex(), m_layoutsTable.rowCount(), m_layoutsTable.rowCount());
    m_layoutsTable << layout;
    endInsertRows();

    emit rowsInserted();
}

void Layouts::applyCurrentNames()
{
    QVector<int> roles;
    roles << Qt::DisplayRole;

    for(int i=0; i<rowCount(); ++i) {
        m_layoutsTable[i].setOriginalName(m_layoutsTable[i].currentName());
    }

    emit dataChanged(index(0, NAMECOLUMN), index(rowCount()-1,NAMECOLUMN), roles);
}

void Layouts::removeLayout(const QString &id)
{
    int index = m_layoutsTable.indexOf(id);

    if (index >= 0) {
        beginRemoveRows(QModelIndex(), index, index);
        m_layoutsTable.remove(index);
        endRemoveRows();
    }
}

bool Layouts::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)

    int firstRow = row;
    int lastRow = row+count-1;

    if (count > 0 && m_layoutsTable.rowExists(firstRow) && (m_layoutsTable.rowExists(lastRow))) {
        bool freeActivitiesLayoutIsRemoved{false};

        for(int i=firstRow; i<=lastRow; ++i) {
            if (m_layoutsTable[i].activities.contains(Data::Layout::FREEACTIVITIESID)) {
                //! we need to reassign it properly
                freeActivitiesLayoutIsRemoved = true;
                break;
            }
        }

        beginRemoveRows(QModelIndex(), firstRow, lastRow);
        for(int i=0; i<count; ++i) {
            m_layoutsTable.remove(firstRow);
        }
        endRemoveRows();

        if (freeActivitiesLayoutIsRemoved) {
            autoAssignFreeActivitiesLayout();
        }

        return true;
    }

    return false;
}

QString Layouts::layoutNameForFreeActivities() const
{
    for(int i=0; i<rowCount(); ++i) {
        if (m_layoutsTable[i].activities.contains(Data::Layout::FREEACTIVITIESID)) {
            return m_layoutsTable[i].currentName();
        }
    }

    return QString();
}

void Layouts::setLayoutNameForFreeActivities(const QString &name)
{
    QString id = m_layoutsTable.idForCurrentName(name);

    if (!id.isEmpty()) {
        m_layoutsTable.setLayoutForFreeActivities(id);

        QVector<int> roles;
        roles << Qt::DisplayRole;
        roles << Qt::UserRole;
        emit dataChanged(index(0, ACTIVITYCOLUMN), index(rowCount()-1, ACTIVITYCOLUMN), roles);
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
            return QString("#");//(i18nc("column for layout background", "Background"));
        } else if (role == Qt::DecorationRole) {
            return QString();//QIcon::fromTheme("games-config-background");
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
    } else if (role == INMULTIPLELAYOUTSROLE) {
        return inMultipleMode();
    } else if (role == LAYOUTNAMEWASEDITEDROLE) {
        return m_layoutsTable[row].nameWasEdited();
    } else if (role == ALLACTIVITIESROLE) {
        QStringList activities;
        activities << QString(Data::Layout::FREEACTIVITIESID);
        activities << m_corona->layoutsManager()->synchronizer()->activities();
        return activities;
    } else if (role == ALLLAYOUTSROLE) {
        QVariant layouts;
        layouts.setValue(m_layoutsTable);
        return layouts;
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
        if (role == Qt::BackgroundRole) {
            return m_layoutsTable[row].background.isEmpty() ? m_layoutsTable[row].color : m_layoutsTable[row].background;
        }
        break;
    case NAMECOLUMN:
        if (role == Qt::DisplayRole) {
            return m_layoutsTable[row].currentName();
        }
        break;
    case MENUCOLUMN:
        if (role == Qt::DisplayRole) {
            return m_layoutsTable[row].isShownInMenu ? CheckMark : QVariant{};
        } else if (role == Qt::UserRole) {
            return m_layoutsTable[row].isShownInMenu;
        }
        break;
    case BORDERSCOLUMN:
        if (role == Qt::DisplayRole) {
            return m_layoutsTable[row].hasDisabledBorders ? CheckMark : QVariant{};
        } else if (role == Qt::UserRole) {
            return m_layoutsTable[row].hasDisabledBorders;
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

QStringList Layouts::cleanStrings(const QStringList &original, const QStringList &occupied)
{
    QStringList result;

    for(int i=0; i<original.count(); ++i) {
        if (!occupied.contains(original[i])) {
            result << original[i];
        }
    }

    return result;
}


void Layouts::autoAssignFreeActivitiesLayout()
{
    QVector<int> roles;
    roles << Qt::DisplayRole;
    roles << Qt::UserRole;

    //! ActiveCurrent with no activities has highest priority
    QString activeCurrentId = m_layoutsTable.idForOriginalName(m_corona->layoutsManager()->currentLayoutName());
    int row = m_layoutsTable.indexOf(activeCurrentId);

    if (row>=0 && m_layoutsTable[row].activities.isEmpty()) {
        m_layoutsTable[row].activities << Data::Layout::FREEACTIVITIESID;
        emit dataChanged(index(row,ACTIVITYCOLUMN), index(row,ACTIVITYCOLUMN), roles);
        return;
    }

    //! Active layouts with no activities have mid priority
    for(int i=0; i<rowCount(); ++i) {
        if (m_layoutsTable[i].isActive && m_layoutsTable[i].activities.isEmpty()) {
            m_layoutsTable[i].activities << Data::Layout::FREEACTIVITIESID;
            emit dataChanged(index(i,ACTIVITYCOLUMN), index(i,ACTIVITYCOLUMN), roles);
            return;
        }
    }

    //! Inactive layouts with no activities have lowest priority
    for(int i=0; i<rowCount(); ++i) {
        if (!m_layoutsTable[i].isActive && m_layoutsTable[i].activities.isEmpty()) {
            m_layoutsTable[i].activities << Data::Layout::FREEACTIVITIESID;
            emit dataChanged(index(i,ACTIVITYCOLUMN), index(i,ACTIVITYCOLUMN), roles);
            return;
        }
    }
}

void Layouts::setActivities(const int &row, const QStringList &activities)
{
    if (!m_layoutsTable.rowExists(row) || m_layoutsTable[row].activities == activities) {
        return;
    }

    QVector<int> roles;
    roles << Qt::DisplayRole;
    roles << Qt::UserRole;

    bool freeActivitiesLayoutIsMissing{false};

    if (m_layoutsTable[row].activities.contains(Data::Layout::FREEACTIVITIESID)
            && !activities.contains(Data::Layout::FREEACTIVITIESID)) {
        //! we need to reassign it properly
        freeActivitiesLayoutIsMissing = true;
    }

    m_layoutsTable[row].activities = activities;
    emit dataChanged(index(row, ACTIVITYCOLUMN), index(row,ACTIVITYCOLUMN), roles);

    for(int i=0; i<rowCount(); ++i) {
        if (i == row) {
            continue;
        }

        auto cleaned = cleanStrings(m_layoutsTable[i].activities, activities);
        if (cleaned != m_layoutsTable[i].activities) {
            m_layoutsTable[i].activities = cleaned;
            emit dataChanged(index(i,ACTIVITYCOLUMN), index(i,ACTIVITYCOLUMN), roles);
        }
    }

    if (freeActivitiesLayoutIsMissing) {
        autoAssignFreeActivitiesLayout();
    }
}

void Layouts::setId(const int &row, const QString &newId)
{
    if (!m_layoutsTable.rowExists(row) || newId.isEmpty() || m_layoutsTable[row].id == newId) {
        return;
    }

    QVector<int> roles;
    roles << Qt::DisplayRole;

    QString oldId = m_layoutsTable[row].id;
    m_layoutsTable[row].id = newId;
    emit dataChanged(index(row, NAMECOLUMN), index(row,NAMECOLUMN), roles);

    for(int i=0; i<rowCount(); ++i) {
        if (i == row) {
            continue;
        }

        int pos = m_layoutsTable[i].shares.indexOf(oldId);

        if (pos >= 0) {
            m_layoutsTable[i].shares[pos] = newId;
            emit dataChanged(index(i, NAMECOLUMN), index(i, NAMECOLUMN), roles);
        }
    }
}

void Layouts::setShares(const int &row, const QStringList &shares)
{
    if (!m_layoutsTable.rowExists(row) || m_layoutsTable[row].shares == shares) {
        return;
    }

    QVector<int> roles;
    roles << Qt::DisplayRole;
    roles << Qt::UserRole;

    m_layoutsTable[row].shares = shares;
    emit dataChanged(index(row,SHAREDCOLUMN), index(row,SHAREDCOLUMN), roles);

    for(int i=0; i<rowCount(); ++i) {
        if (i == row) {
            continue;
        }

        auto cleaned = cleanStrings(m_layoutsTable[i].shares, shares);
        if (cleaned != m_layoutsTable[i].shares) {
            m_layoutsTable[i].shares = cleaned;
            emit dataChanged(index(i,SHAREDCOLUMN), index(i,SHAREDCOLUMN), roles);
        }
    }
}

bool Layouts::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const int row = index.row();
    const int column = index.column();

    if (!m_layoutsTable.rowExists(row) || column<0 || column > SHAREDCOLUMN) {
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
            setId(row, value.toString());
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
            QString provenId = m_layoutsTable.idForCurrentName(value.toString());

            if (!provenId.isEmpty() && provenId != m_layoutsTable[row].id /*not the same row*/ ){
                //! duplicate name should be rejected
                emit nameDuplicated(provenId, m_layoutsTable[row].id);
                return false;
            } else {
                m_layoutsTable[row].setCurrentName(value.toString());
                emit dataChanged(index, index, roles);
                return true;
            }
        }
        break;
    case MENUCOLUMN:
        if (role == Qt::DisplayRole || role == Qt::UserRole) {
            m_layoutsTable[row].isShownInMenu = value.toBool();
            emit dataChanged(index, index, roles);
            return true;
        }
        break;
    case BORDERSCOLUMN:
        if (role == Qt::DisplayRole || role == Qt::UserRole) {
            m_layoutsTable[row].hasDisabledBorders = value.toBool();
            emit dataChanged(index, index, roles);
            return true;
        }
        break;
    case ACTIVITYCOLUMN:
        if (role == Qt::UserRole) {
            setActivities(row, value.toStringList());
            return true;
        }
        break;
    case SHAREDCOLUMN:
        if (role == Qt::UserRole) {
            setShares(row, value.toStringList());
            return true;
        }
        break;
    };

    return false;
}

void Layouts::updateActiveStates()
{
    QVector<int> roles;
    roles << Qt::DisplayRole;
    roles << Qt::UserRole;
    roles << LAYOUTISACTIVEROLE;

    for(int i=0; i<rowCount(); ++i) {
        bool iActive{false};

        if (m_corona->layoutsManager()->synchronizer()->layout(m_layoutsTable[i].currentName())) {
            iActive = true;
        }

        if (m_layoutsTable[i].isActive != iActive) {
            m_layoutsTable[i].isActive = iActive;

            emit dataChanged(index(i, BACKGROUNDCOLUMN), index(i,SHAREDCOLUMN), roles);
        }
    }
}

const Data::Layout &Layouts::at(const int &row)
{
    return m_layoutsTable[row];
}

int Layouts::rowForId(const QString &id) const
{
    return m_layoutsTable.indexOf(id);
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

    emit rowsInserted();
}

}
}
}
