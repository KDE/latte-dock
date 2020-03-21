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
#include "../../settings/universalsettings.h"

// Qt
#include <QDebug>
#include <QFont>
#include <QIcon>

// KDE
#include <KLocalizedString>

// KActivities
#include <KActivities/Consumer>
#include <KActivities/Info>

namespace Latte {
namespace Settings {
namespace Model {

Layouts::Layouts(QObject *parent, Latte::Corona *corona)
    : QAbstractTableModel(parent),
      m_corona(corona)
{
    initActivities();

    connect(this, &Layouts::inMultipleModeChanged, this, [&]() {
        QVector<int> roles;
        roles << Qt::DisplayRole;
        roles << Qt::UserRole;
        roles << ISSHAREDROLE;
        roles << INMULTIPLELAYOUTSROLE;

        emit dataChanged(index(0, NAMECOLUMN), index(rowCount()-1, SHAREDCOLUMN), roles);
    });

    connect(this, &Layouts::inMultipleModeChanged, this, &Layouts::updateActiveStates);
    connect(m_corona->layoutsManager(), &Latte::Layouts::Manager::currentLayoutNameChanged, this, &Layouts::updateActiveStates);
    connect(m_corona->layoutsManager(), &Latte::Layouts::Manager::centralLayoutsChanged, this, &Layouts::updateActiveStates);

    connect(m_corona->universalSettings(), &Latte::UniversalSettings::lastNonAssignedLayoutNameChanged, this, [&]() {
        //FREE ACTIVITES LAYOUT changed and our model must be updated...
        assignFreeActivitiesLayoutAt(m_corona->universalSettings()->lastNonAssignedLayoutName());
    });
}

Layouts::~Layouts()
{
    qDeleteAll(m_activitiesInfo);
}

bool Layouts::containsCurrentName(const QString &name) const
{
    return m_layoutsTable.containsName(name);
}

bool Layouts::dataAreChanged() const
{
    return ((o_inMultipleMode != m_inMultipleMode) || (o_layoutsTable != m_layoutsTable));
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

void Layouts::applyData()
{   
    QVector<int> roles;
    roles << Qt::DisplayRole;
    roles << Qt::UserRole;

    o_inMultipleMode = m_inMultipleMode;
    o_layoutsTable = m_layoutsTable;

    emit dataChanged(index(0, BACKGROUNDCOLUMN), index(rowCount()-1,SHAREDCOLUMN), roles);
}

void Layouts::resetData()
{
    bool appending{false};
    bool removing{false};

    if (m_layoutsTable.rowCount() > o_layoutsTable.rowCount()) {
        removing = true;
        beginRemoveRows(QModelIndex(), m_layoutsTable.rowCount(), o_layoutsTable.rowCount()-1);
    } else if (m_layoutsTable.rowCount() < o_layoutsTable.rowCount()){
        appending = true;
        beginInsertRows(QModelIndex(), m_layoutsTable.rowCount(), o_layoutsTable.rowCount()-1);
    }

    QVector<int> roles;
    roles << Qt::DisplayRole;
    roles << Qt::UserRole;

    m_inMultipleMode = o_inMultipleMode;
    m_layoutsTable = o_layoutsTable;

    if (appending) {
        endInsertRows();
    } else if (removing) {
        endRemoveRows();
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
            return m_layoutsTable[i].name;
        }
    }

    return QString();
}

void Layouts::setCurrentLayoutForFreeActivities(const QString &id)
{
    if (m_layoutsTable.containsId(id)) {
        m_layoutsTable.setLayoutForFreeActivities(id);

        QVector<int> roles;
        roles << Qt::DisplayRole;
        roles << Qt::UserRole;
        emit dataChanged(index(0, ACTIVITYCOLUMN), index(rowCount()-1, ACTIVITYCOLUMN), roles);
    }
}

void Layouts::setOriginalLayoutForFreeActivities(const QString &id)
{
    if (o_layoutsTable.containsId(id)) {
        o_layoutsTable.setLayoutForFreeActivities(id);
        m_layoutsTable.setLayoutForFreeActivities(id);

        QVector<int> roles;
        roles << Qt::DisplayRole;
        roles << Qt::UserRole;
        emit dataChanged(index(0, ACTIVITYCOLUMN), index(rowCount()-1, ACTIVITYCOLUMN), roles);
    }
}

QStringList Layouts::assignedActivitiesFromShared(const int &row) const
{
    QStringList assigns;

    if (!m_layoutsTable.rowExists(row)) {
        return assigns;
    }

    if (m_layoutsTable[row].isShared()) {
        for (int i=0; i<m_layoutsTable[row].shares.count(); ++i) {
            QString shareId = m_layoutsTable[row].shares[i];
            int shareRow = rowForId(shareId);

            if (shareRow>=0 && !m_layoutsTable[shareRow].activities.isEmpty()) {
                assigns << m_layoutsTable[shareRow].activities;
            }
        }
    }

    return assigns;
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
            return QString("");
        }
        break;
    case BACKGROUNDCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString("");
        } else if (role == Qt::DecorationRole) {
            return QIcon::fromTheme("games-config-background");
        } else if (role == Qt::TextAlignmentRole ){
            return QVariant::fromValue(Qt::AlignHCenter | Qt::AlignVCenter);
        }
        break;
    case NAMECOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("column for layout name", "Name"));
        }/* else if (role == Qt::TextAlignmentRole) {
            return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
        }*/
        break;
    case MENUCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("column for layout to show in menu", "In Menu"));
        }/* else if (role == Qt::TextAlignmentRole ){
            return QVariant::fromValue(Qt::AlignHCenter | Qt::AlignVCenter);
        }*/
        break;
    case BORDERSCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("column for layout to hide borders for maximized windows", "Borderless"));
        }/* else if (role == Qt::TextAlignmentRole ){
            return QVariant::fromValue(Qt::AlignHCenter | Qt::AlignVCenter);
        }*/
        break;
    case ACTIVITYCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("column for layout to show which activities is assigned to", "Activities"));
        } else if (role == Qt::DecorationRole) {
            return QIcon::fromTheme("activities");
        }/* else if (role == Qt::TextAlignmentRole ){
            return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
        }*/
        break;
    case SHAREDCOLUMN:
        if (role == Qt::DisplayRole) {
            return QString(i18nc("column for shared layout to show which layouts is assigned to", "Shared To"));
        } else if (role == Qt::DecorationRole) {
            return QIcon::fromTheme("document-share");
        }/* else if (role == Qt::TextAlignmentRole ){
            return QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter);
        }*/
        break;
    default:
        break;
    };

    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags Layouts::flags(const QModelIndex &index) const
{
    const int column = index.column();
    const int row = index.row();

    auto flags = QAbstractTableModel::flags(index);

    if (column == MENUCOLUMN || column == BORDERSCOLUMN) {
        flags |= Qt::ItemIsUserCheckable;
    }

    if (column == ACTIVITYCOLUMN
            || column == BACKGROUNDCOLUMN
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
    bool isNewLayout = !o_layoutsTable.containsId(m_layoutsTable[row].id);

    if (!m_layoutsTable.rowExists(row)) {
        return QVariant{};
    }

    if (role == IDROLE) {
        return m_layoutsTable[row].id;
    } else if (role == ISACTIVEROLE) {
        return m_layoutsTable[row].isActive;
    } else if (role == ISLOCKEDROLE) {
        return m_layoutsTable[row].isLocked;
    } else if (role == ISSHAREDROLE) {
        return m_layoutsTable[row].isShared();
    } else if (role == INMULTIPLELAYOUTSROLE) {
        return inMultipleMode();
    } else if (role == ASSIGNEDACTIVITIESROLE) {
        return m_layoutsTable[row].activities;
    } else if (role == ASSIGNEDACTIVITIESFROMSHAREDROLE) {
        return assignedActivitiesFromShared(row);
    } else if (role == ALLACTIVITIESSORTEDROLE) {
        QStringList activities;
        activities << QString(Data::Layout::FREEACTIVITIESID);
        activities << m_corona->layoutsManager()->synchronizer()->activities();
        return activities;
    } else if (role == ALLACTIVITIESDATAROLE) {
        QVariant activitiesData;
        activitiesData.setValue(m_activitiesMap);
        return activitiesData;
    } else if (role == ALLLAYOUTSROLE) {
        QVariant layouts;
        layouts.setValue(m_layoutsTable);
        return layouts;
    } else if (role == SHAREDTOINEDIT) {
        return (m_sharedToInEditRow == row);
    } else if (role == ISNEWLAYOUTROLE) {
        return isNewLayout;
    } else if (role == LAYOUTHASCHANGESROLE) {
        return (isNewLayout ? true : o_layoutsTable[m_layoutsTable[row].id] != m_layoutsTable[row]);
    }

    switch (column) {
    case IDCOLUMN:
        if (role == Qt::DisplayRole || role == Qt::UserRole){
            return m_layoutsTable[row].id;
        }
        break;
    case HIDDENTEXTCOLUMN:
        return QVariant{};
    case BACKGROUNDCOLUMN:
        if (role == SORTINGROLE) {
            return m_layoutsTable[row].name;
        }

        if (role == Qt::UserRole) {
            return m_layoutsTable[row].background.isEmpty() ? m_layoutsTable[row].color : m_layoutsTable[row].background;
        }
        break;
    case NAMECOLUMN:
        if (role == SORTINGROLE) {
            return m_layoutsTable[row].name;
        }

        if ((role == Qt::DisplayRole) || (role == Qt::UserRole)) {
            return m_layoutsTable[row].name;
        }
        break;
    case MENUCOLUMN:
        if (role == SORTINGROLE) {
            if ((m_inMultipleMode && m_layoutsTable[row].isShared())) {
                return 1;
            } else if (m_layoutsTable[row].isShownInMenu) {
                return 2;
            }

            return 0;
        }

        if (role == ORIGINALISSHOWNINMENUROLE) {
            return isNewLayout ? false : o_layoutsTable[row].isShownInMenu;
        }

        if (role == Qt::UserRole) {
            return m_layoutsTable[row].isShownInMenu;
        }
        break;
    case BORDERSCOLUMN:
        if (role == SORTINGROLE) {
            if ((m_inMultipleMode && m_layoutsTable[row].isShared())) {
                return 1;
            } else if (m_layoutsTable[row].hasDisabledBorders) {
                return 2;
            }

            return 0;
        }

        if (role == ORIGINALHASBORDERSROLE) {
            return isNewLayout ? false : o_layoutsTable[row].hasDisabledBorders;
        }

        if (role == Qt::UserRole) {
            return m_layoutsTable[row].hasDisabledBorders;
        }
        break;
    case ACTIVITYCOLUMN:
        if (role == SORTINGROLE) {
            if ((m_inMultipleMode && m_layoutsTable[row].isShared())) {
                //! normal priority
                return 1;
            } else if (m_layoutsTable[row].activities.count() > 0) {
                if (m_layoutsTable[row].activities.contains(Data::Layout::FREEACTIVITIESID)) {
                    //! highest priority
                    return 9999;
                } else {
                    //! high priority
                    return m_layoutsTable[row].activities.count()+1;
                }
            }

            return 0;
        }

        if (role == ORIGINALASSIGNEDACTIVITIESROLE) {
            return isNewLayout ? QStringList() : o_layoutsTable[row].activities;
        }

        if (role == Qt::UserRole) {
            return m_layoutsTable[row].activities;
        }
        break;
    case SHAREDCOLUMN:
        if (role == SORTINGROLE) {
            if (m_layoutsTable[row].shares.count() > 0) {
                //! highest priority based on number of shares
                return 9999 + m_layoutsTable[row].shares.count();
            }

            if (m_layoutsTable[row].activities.contains(Data::Layout::FREEACTIVITIESID)) {
                //! high activity priority
                return 9998;
            }

            return 0;
        }

        if (role == ORIGINALSHARESROLE) {
            return isNewLayout ? QStringList() : o_layoutsTable[row].shares;
        }

        if (role == Qt::UserRole) {
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


void Layouts::assignFreeActivitiesLayoutAt(const QString &layoutName)
{
    QString reqId = o_layoutsTable.idForName(layoutName);

    if (reqId.isEmpty()) {
        reqId = m_layoutsTable.idForName(layoutName);
        if (reqId.isEmpty()) {
            return;
        }
    }

    int row = m_layoutsTable.indexOf(reqId);
    setActivities(row, QStringList(Data::Layout::FREEACTIVITIESID));
    setShares(row, QStringList());
}

void Layouts::autoAssignFreeActivitiesLayout()
{
    QVector<int> roles;
    roles << Qt::DisplayRole;
    roles << Qt::UserRole;

    //! ActiveCurrent with no activities has highest priority
    QString activeCurrentId = o_layoutsTable.idForName(m_corona->layoutsManager()->currentLayoutName());
    int row = m_layoutsTable.indexOf(activeCurrentId);

    if (row>=0 && !(m_inMultipleMode && m_layoutsTable[row].isShared()) && m_layoutsTable[row].activities.isEmpty()) {
        m_layoutsTable[row].activities << Data::Layout::FREEACTIVITIESID;
        emit dataChanged(index(row,BACKGROUNDCOLUMN), index(row,ACTIVITYCOLUMN), roles);
        return;
    }

    //! Active layouts with no activities have mid priority
    for(int i=0; i<rowCount(); ++i) {
        if (m_layoutsTable[i].isActive && m_layoutsTable[i].activities.isEmpty() && !(m_inMultipleMode && m_layoutsTable[i].isShared())) {
            m_layoutsTable[i].activities << Data::Layout::FREEACTIVITIESID;
            emit dataChanged(index(i,BACKGROUNDCOLUMN), index(i,ACTIVITYCOLUMN), roles);
            return;
        }
    }

    //! Inactive layouts with no activities have lowest priority
    for(int i=0; i<rowCount(); ++i) {
        if (!m_layoutsTable[i].isActive && m_layoutsTable[i].activities.isEmpty() && !(m_inMultipleMode && m_layoutsTable[i].isShared())) {
            m_layoutsTable[i].activities << Data::Layout::FREEACTIVITIESID;
            emit dataChanged(index(i,BACKGROUNDCOLUMN), index(i,ACTIVITYCOLUMN), roles);
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
    roles << ASSIGNEDACTIVITIESROLE;

    bool freeActivitiesLayoutIsMissing{false};

    if (m_layoutsTable[row].activities.contains(Data::Layout::FREEACTIVITIESID)
            && !activities.contains(Data::Layout::FREEACTIVITIESID)) {
        //! we need to reassign it properly
        freeActivitiesLayoutIsMissing = true;
    }

    m_layoutsTable[row].activities = activities;
    emit dataChanged(index(row, BACKGROUNDCOLUMN), index(row,ACTIVITYCOLUMN), roles);

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

QStringList Layouts::availableShareIdsFor(const QString id) const
{
    QStringList shares;

    for(int i=0; i<rowCount(); ++i) {
        if (m_layoutsTable[i].id == id || m_layoutsTable[i].isShared()) {
            continue;
        }

        shares << m_layoutsTable[i].id;
    }

    return shares;
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
    emit dataChanged(index(row,IDCOLUMN), index(row,SHAREDCOLUMN), roles);

    for(int i=0; i<rowCount(); ++i) {
        if (i == row) {
            continue;
        }

        auto cleaned = cleanStrings(m_layoutsTable[i].shares, shares);
        if (cleaned != m_layoutsTable[i].shares) {
            m_layoutsTable[i].shares = cleaned;
            emit dataChanged(index(i,IDCOLUMN), index(i,SHAREDCOLUMN), roles);
        }
    }

    if (m_layoutsTable[row].activities.contains(Data::Layout::FREEACTIVITIESID)
            && m_inMultipleMode
            && m_layoutsTable[row].isShared()) {
        //! we need to remove the free_activities flag in such case
        setActivities(row, QStringList());
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
    if (role == ISLOCKEDROLE) {
        m_layoutsTable[row].isLocked = value.toBool();
        emit dataChanged(this->index(row,0), this->index(row,SHAREDCOLUMN), roles);
        return true;
    }

    //! specific roles to each independent cell
    switch (column) {
    case IDCOLUMN:
        if (role==Qt::UserRole) {
            setId(row, value.toString());
            emit dataChanged(index, index, roles);
            return true;
        }
        break;
    case HIDDENTEXTCOLUMN:
        return true;
        break;
    case BACKGROUNDCOLUMN:
        if (role == Qt::UserRole) {
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
        if (role == Qt::UserRole) {
            QString provenId = m_layoutsTable.idForName(value.toString());

            if (!provenId.isEmpty() && provenId != m_layoutsTable[row].id /*not the same row*/ ){
                //! duplicate name should be rejected
                emit nameDuplicated(provenId, m_layoutsTable[row].id);
                return false;
            } else {
                m_layoutsTable[row].name = value.toString();
                emit dataChanged(index, index, roles);
                return true;
            }
        }
        break;
    case MENUCOLUMN:
        if (role == Qt::UserRole) {
            m_layoutsTable[row].isShownInMenu = value.toBool();
            emit dataChanged(index, index, roles);
            emit dataChanged(this->index(row, NAMECOLUMN), this->index(row,NAMECOLUMN), roles);
            return true;
        }
        break;
    case BORDERSCOLUMN:
        if (role == Qt::UserRole) {
            m_layoutsTable[row].hasDisabledBorders = value.toBool();
            emit dataChanged(index, index, roles);
            emit dataChanged(this->index(row, NAMECOLUMN), this->index(row,NAMECOLUMN), roles);
            return true;
        }
        break;
    case ACTIVITYCOLUMN:
        if (role == Qt::UserRole) {
            setActivities(row, value.toStringList());
            emit dataChanged(this->index(row, NAMECOLUMN), this->index(row,NAMECOLUMN), roles);
            return true;
        }
        break;
    case SHAREDCOLUMN:
        if (role == Qt::UserRole) {
            setShares(row, value.toStringList());
            emit dataChanged(this->index(row, NAMECOLUMN), this->index(row,NAMECOLUMN), roles);
            return true;
        } else if (role == SHAREDTOINEDIT) {
            bool inEdit = value.toBool();
            m_sharedToInEditRow = inEdit ? row : -1;
            roles << Qt::DisplayRole;
            roles << Qt::UserRole;
            emit dataChanged(this->index(row, ACTIVITYCOLUMN), this->index(row, SHAREDCOLUMN),  roles);
            emit dataChanged(this->index(row, NAMECOLUMN), this->index(row,NAMECOLUMN), roles);
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
    roles << ISACTIVEROLE;

    for(int i=0; i<rowCount(); ++i) {
        bool iActive{false};

        if (m_inMultipleMode && m_corona->layoutsManager()->synchronizer()->layout(m_layoutsTable[i].name)
                || (!m_inMultipleMode && o_layoutsTable[i].name == m_corona->layoutsManager()->synchronizer()->currentLayoutName())) {
            iActive = true;
        }

        if (m_layoutsTable[i].isActive != iActive) {
            m_layoutsTable[i].isActive = iActive;

            emit dataChanged(index(i, BACKGROUNDCOLUMN), index(i,SHAREDCOLUMN), roles);
        }
    }
}

int Layouts::rowForId(const QString &id) const
{
    return m_layoutsTable.indexOf(id);
}

const Data::Layout &Layouts::at(const int &row)
{
    return m_layoutsTable[row];
}

const Data::Layout &Layouts::currentData(const QString &id)
{
    return m_layoutsTable[id];
}


const Data::Layout Layouts::originalData(const QString &id)
{
    if (o_layoutsTable.containsId(id)){
        return o_layoutsTable[id];
    }

    return Data::Layout();

}

const Data::LayoutsTable &Layouts::originalLayoutsData()
{
    return o_layoutsTable;
}

const Data::LayoutsTable &Layouts::currentLayoutsData()
{
    return m_layoutsTable;
}

void Layouts::setOriginalData(Data::LayoutsTable &data, const bool &inmultiple)
{
    clear();

    beginInsertRows(QModelIndex(), 0, data.rowCount() - 1);
    o_inMultipleMode = inmultiple;
    o_layoutsTable = data;

    m_layoutsTable = data;

    for(int i=0; i<m_layoutsTable.rowCount(); ++i) {
        m_layoutsTable[i].isActive = m_corona->layoutsManager()->synchronizer()->layout(o_layoutsTable[i].name);
    }
    endInsertRows();

    setInMultipleMode(inmultiple);

    emit rowsInserted();
}


//! Activities code
void Layouts::initActivities()
{
    Data::Activity freeActivities;
    freeActivities.id = Data::Layout::FREEACTIVITIESID;
    freeActivities.name = QString("[ " + i18n("All Free Activities...") + " ]");
    freeActivities.icon = "favorites";
    freeActivities.state = KActivities::Info::Stopped;
    m_activitiesMap[Data::Layout::FREEACTIVITIESID] = freeActivities;

    QStringList activities = m_corona->layoutsManager()->synchronizer()->activities();;

    for(const auto &id: activities) {
        KActivities::Info info(id);

        if (info.state() != KActivities::Info::Invalid) {
            on_activityAdded(id);
        }
    }

    connect(m_corona->activitiesConsumer(), &KActivities::Consumer::activityAdded, this, &Layouts::on_activityAdded);
    connect(m_corona->activitiesConsumer(), &KActivities::Consumer::activityRemoved, this, &Layouts::on_activityRemoved);
    connect(m_corona->activitiesConsumer(), &KActivities::Consumer::runningActivitiesChanged, this, &Layouts::on_runningActivitiesChanged);

    activitiesStatesChanged();
}

void Layouts::activitiesStatesChanged()
{
    QVector<int> roles;
    roles << Qt::DisplayRole;
    roles << Qt::UserRole;
    roles << ALLACTIVITIESDATAROLE;
    roles << ALLACTIVITIESSORTEDROLE;

    emit dataChanged(index(0, BACKGROUNDCOLUMN), index(rowCount()-1, BACKGROUNDCOLUMN), roles);
    emit dataChanged(index(0, ACTIVITYCOLUMN), index(rowCount()-1, ACTIVITYCOLUMN), roles);
}

void Layouts::on_activityAdded(const QString &id)
{
    m_activitiesInfo[id] = new KActivities::Info(id, this);

    Data::Activity activity;
    activity.id = m_activitiesInfo[id]->id();
    activity.name = m_activitiesInfo[id]->name();
    activity.icon = m_activitiesInfo[id]->icon();
    activity.state = m_activitiesInfo[id]->state();

    m_activitiesMap[id] = activity;

    connect(m_activitiesInfo[id], &KActivities::Info::nameChanged, [this, id]() {
        on_activityChanged(id);
    });

    connect(m_activitiesInfo[id], &KActivities::Info::iconChanged, [this, id]() {
        on_activityChanged(id);
    });
}

void Layouts::on_activityRemoved(const QString &id)
{
    if (m_activitiesMap.contains(id)) {
        m_activitiesMap.remove(id);
    }

    if (m_activitiesInfo.contains(id)) {
        KActivities::Info *info = m_activitiesInfo.take(id);
        info->deleteLater();
    }

    activitiesStatesChanged();
}

void Layouts::on_activityChanged(const QString &id)
{
    if (m_activitiesMap.contains(id) && m_activitiesInfo.contains(id)) {
        m_activitiesMap[id].name = m_activitiesInfo[id]->name();
        m_activitiesMap[id].icon = m_activitiesInfo[id]->icon();
        m_activitiesMap[id].state = m_activitiesInfo[id]->state();

        activitiesStatesChanged();
    }
}

void Layouts::on_runningActivitiesChanged(const QStringList &runningIds)
{
    Data::ActivitiesMap::iterator i;

    for (i = m_activitiesMap.begin(); i != m_activitiesMap.end(); ++i){
        if (runningIds.contains(i.key())) {
            m_activitiesMap[i.key()].state = KActivities::Info::Running;
        } else {
            m_activitiesMap[i.key()].state = KActivities::Info::Stopped;
        }
    }

    activitiesStatesChanged();
}

}
}
}
