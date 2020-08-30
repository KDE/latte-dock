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
#include "../../data/layoutdata.h"
#include "../../layouts/manager.h"
#include "../../layouts/synchronizer.h"
#include "../../settings/universalsettings.h"

// Qt
#include <QDebug>
#include <QFileInfo>
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
        roles << INMULTIPLELAYOUTSROLE;

        emit dataChanged(index(0, NAMECOLUMN), index(rowCount()-1, ACTIVITYCOLUMN), roles);
    });

    connect(m_corona->layoutsManager(), &Latte::Layouts::Manager::centralLayoutsChanged, this, &Layouts::updateActiveStates);
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
    return modeIsChanged() || layoutsAreChanged();
}

bool Layouts::modeIsChanged() const
{
    return o_inMultipleMode != m_inMultipleMode;
}

bool Layouts::layoutsAreChanged() const
{
    return o_layoutsTable != m_layoutsTable;
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

    return ACTIVITYCOLUMN+1;
}

void Layouts::clear()
{
    if (m_layoutsTable.rowCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, m_layoutsTable.rowCount() - 1);
        m_layoutsTable.clear();
        endRemoveRows();
    }
}

void Layouts::appendLayout(const Latte::Data::Layout &layout)
{
    int newRow = m_layoutsTable.sortedPosForName(layout.name);

    beginInsertRows(QModelIndex(), newRow, newRow);
    m_layoutsTable.insert(newRow, layout);
    endInsertRows();

    emit rowsInserted();
}

void Layouts::appendOriginalLayout(const Latte::Data::Layout &layout)
{
    int newRow = o_layoutsTable.sortedPosForName(layout.name);
    o_layoutsTable.insert(newRow, layout);

    appendLayout(layout);
}

void Layouts::applyData()
{   
    QVector<int> roles;
    roles << Qt::DisplayRole;
    roles << Qt::UserRole;

    o_inMultipleMode = m_inMultipleMode;
    o_layoutsTable = m_layoutsTable;

    emit dataChanged(index(0, BACKGROUNDCOLUMN), index(rowCount()-1, ACTIVITYCOLUMN), roles);
}

void Layouts::resetData()
{
    clear();
    setOriginalInMultipleMode(o_inMultipleMode);
    setOriginalData(o_layoutsTable);
}

void Layouts::removeLayout(const QString &id)
{
    int index = m_layoutsTable.indexOf(id);

    if (index >= 0) {
        removeRows(index,1);
    }
}

void Layouts::setLayoutProperties(const Latte::Data::Layout &layout)
{
    if (m_layoutsTable.containsId(layout.id) && m_layoutsTable[layout.id] != layout) {
        m_layoutsTable[layout.id] = layout;
        int dataRow = m_layoutsTable.indexOf(layout.id);

        QVector<int> roles;
        roles << Qt::DisplayRole;
        roles << Qt::UserRole;
        emit dataChanged(index(dataRow, IDCOLUMN), index(dataRow, ACTIVITYCOLUMN), roles);
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
            if (m_layoutsTable[i].activities.contains(Latte::Data::Layout::FREEACTIVITIESID)) {
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

        return true;
    }

    return false;
}

QString Layouts::layoutNameForFreeActivities() const
{
    for(int i=0; i<rowCount(); ++i) {
        if (m_layoutsTable[i].activities.contains(Latte::Data::Layout::FREEACTIVITIESID)) {
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
            || column == NAMECOLUMN) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

void Layouts::setIconsPath(QString iconsPath)
{
    m_iconsPath = iconsPath;
}

QList<Latte::Data::LayoutIcon> Layouts::iconsForCentralLayout(const int &row) const
{
    QList<Latte::Data::LayoutIcon> icons;

    if (inMultipleMode()) {
        if (m_layoutsTable[row].activities.contains(Latte::Data::Layout::ALLACTIVITIESID)) {
            Latte::Data::LayoutIcon icon;
            icon.name = m_activitiesTable[Latte::Data::Layout::ALLACTIVITIESID].icon;
            icon.isBackgroundFile = false;
            icons << icon;
        } else if (m_layoutsTable[row].activities.contains(Latte::Data::Layout::FREEACTIVITIESID)) {
            Latte::Data::LayoutIcon icon;
            icon.name = m_activitiesTable[Latte::Data::Layout::FREEACTIVITIESID].icon;
            icon.isBackgroundFile = false;
            icons << icon;
        } else {
            QStringList activitiesIds = m_layoutsTable[row].activities;

            for(int i=0; i<activitiesIds.count(); ++i) {
                QString id = activitiesIds[i];
                if (m_activitiesTable.containsId(id)) {
                    Latte::Data::LayoutIcon icon;
                    icon.name = m_activitiesTable[id].icon;
                    icon.isBackgroundFile = false;
                    icons << icon;
                }
            }
        }
    } else {
        if (o_layoutsTable.containsId(m_layoutsTable[row].id) && o_layoutsTable[m_layoutsTable[row].id].name == m_corona->universalSettings()->singleModeLayoutName()) {
            Latte::Data::LayoutIcon icon;
            icon.name = m_activitiesTable[Latte::Data::Layout::ALLACTIVITIESID].icon;
            icon.isBackgroundFile = false;
            icons << icon;
        }
    }

    if (!m_layoutsTable[row].icon.isEmpty()) {
        //! if there is specific icon set from the user for this layout we draw only that icon
        icons.clear();
        Latte::Data::LayoutIcon icon;
        icon.name = m_layoutsTable[row].icon;
        icon.isBackgroundFile = false;
        icons << icon;
        return icons;
    }

    //! background image
    if (icons.count() == 0) {
        QString colorPath;

        if (m_layoutsTable[row].backgroundStyle == Layout::PatternBackgroundStyle && m_layoutsTable[row].background.isEmpty()) {
            colorPath = m_iconsPath + "defaultcustomprint.jpg";
        } else {
            colorPath = m_layoutsTable[row].background.startsWith("/") ? m_layoutsTable[row].background : m_iconsPath + m_layoutsTable[row].color + "print.jpg";
        }

        if (QFileInfo(colorPath).exists()) {
            Latte::Data::LayoutIcon icon;
            icon.isBackgroundFile = true;
            icon.name = colorPath;
            icons << icon;
        }
    }

    return icons;
}

QList<Latte::Data::LayoutIcon> Layouts::icons(const int &row) const
{
    return iconsForCentralLayout(row);
}

QString Layouts::sortableText(const int &priority, const int &row) const
{
    QString numberPart;

    if (priority < 10) {
        numberPart = "00000" + QString::number(priority);
    } else if (priority < 100) {
        numberPart = "0000" + QString::number(priority);
    } else if (priority < 1000) {
        numberPart = "000" + QString::number(priority);
    } else if (priority < 10000) {
        numberPart = "00" + QString::number(priority);
    } else if (priority < 100000) {
        numberPart = "0" + QString::number(priority);
    }

    return (numberPart + m_layoutsTable[row].name);
}


QString Layouts::sortingPriority(const SortingPriority &priority, const int &row) const
{
    int iPriority = (int)priority;

    iPriority = (m_layoutsTable[row].isActive && inMultipleMode() ? iPriority - 1000 : iPriority);

    return sortableText(iPriority, row);
}

QVariant Layouts::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    int column = index.column();
    bool isNewLayout = !o_layoutsTable.containsId(m_layoutsTable[row].id);

    if (!m_layoutsTable.rowExists(row)) {
        return QVariant{};
    }

    //! original data
    Latte::Data::Layout original;

    if (!isNewLayout) {
        original = o_layoutsTable[m_layoutsTable[row].id];
    }

    if (role == IDROLE) {
        return m_layoutsTable[row].id;
    } else if (role == ISACTIVEROLE) {
        return m_layoutsTable[row].isActive;
    } else if (role == ISLOCKEDROLE) {
        return m_layoutsTable[row].isLocked;
    } else if (role == INMULTIPLELAYOUTSROLE) {
        return inMultipleMode();
    } else if (role == ASSIGNEDACTIVITIESROLE) {
        return m_layoutsTable[row].activities;
    } else if (role == ALLACTIVITIESSORTEDROLE) {
        QStringList activities;
        activities << QString(Latte::Data::Layout::ALLACTIVITIESID);
        activities << QString(Latte::Data::Layout::FREEACTIVITIESID);
        activities << QString(Latte::Data::Layout::CURRENTACTIVITYID);
        activities << m_corona->layoutsManager()->synchronizer()->activities();
        return activities;
    } else if (role == ALLACTIVITIESDATAROLE) {
        QVariant activitiesData;
        activitiesData.setValue(m_activitiesTable);
        return activitiesData;
    } else if (role == ALLLAYOUTSROLE) {
        QVariant layouts;
        layouts.setValue(m_layoutsTable);
        return layouts;
    } else if (role == ISNEWLAYOUTROLE) {
        return isNewLayout;
    } else if (role == LAYOUTHASCHANGESROLE) {
        return isNewLayout ? true : (original != m_layoutsTable[row]);
    } else if (role == BACKGROUNDUSERROLE) {
        QList<Latte::Data::LayoutIcon> iconsList = icons(row);
        QVariant iconsVariant;
        iconsVariant.setValue<QList<Latte::Data::LayoutIcon>>(iconsList);
        return iconsVariant;
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

        if (role == Qt::DisplayRole) {
            return m_layoutsTable[row].background;
        } else if (role == Qt::UserRole) {
            QList<Latte::Data::LayoutIcon> iconsList = icons(row);
            QVariant iconsVariant;
            iconsVariant.setValue<QList<Latte::Data::LayoutIcon>>(iconsList);
            return iconsVariant;
        }
        break;
    case NAMECOLUMN:
        if (role == SORTINGROLE) {
            if ((inMultipleMode() && m_layoutsTable[row].isActive)
                    || (!inMultipleMode()
                        && !original.name.isEmpty()
                        && original.name == m_corona->universalSettings()->singleModeLayoutName())) {
                return sortingPriority(HIGHESTPRIORITY, row);
            }

            return sortingPriority(NORMALPRIORITY, row);
        }

        if ((role == Qt::DisplayRole) || (role == Qt::UserRole)) {
            return m_layoutsTable[row].name;
        }
        break;
    case MENUCOLUMN:
        if (role == SORTINGROLE) {
            if (m_layoutsTable[row].isShownInMenu) {
                return sortingPriority(HIGHESTPRIORITY, row);
            }

            return sortingPriority(NORMALPRIORITY, row);
        }

        if (role == ORIGINALISSHOWNINMENUROLE) {
            return isNewLayout ? false : original.isShownInMenu;
        }

        if (role == Qt::UserRole) {
            return m_layoutsTable[row].isShownInMenu;
        }
        break;
    case BORDERSCOLUMN:
        if (role == SORTINGROLE) {
            if (m_layoutsTable[row].hasDisabledBorders) {
                return sortingPriority(HIGHESTPRIORITY, row);
            }

            return sortingPriority(NORMALPRIORITY, row);
        }

        if (role == ORIGINALHASBORDERSROLE) {
            return isNewLayout ? false : original.hasDisabledBorders;
        }

        if (role == Qt::UserRole) {
            return m_layoutsTable[row].hasDisabledBorders;
        }
        break;
    case ACTIVITYCOLUMN:
        if (role == SORTINGROLE) {
            if (m_layoutsTable[row].activities.count() > 0) {
                if (m_layoutsTable[row].activities.contains(Latte::Data::Layout::ALLACTIVITIESID)) {
                    return sortingPriority(HIGHESTPRIORITY, row);
                } else if (m_layoutsTable[row].activities.contains(Latte::Data::Layout::FREEACTIVITIESID)) {
                    return sortingPriority(HIGHPRIORITY, row);
                } else {
                    return sortingPriority(MEDIUMPRIORITY, row) + m_layoutsTable[row].activities.count();
                }
            }

            return sortingPriority(NORMALPRIORITY, row) + m_layoutsTable[row].activities.count();
        }

        if (role == ORIGINALASSIGNEDACTIVITIESROLE) {
            return isNewLayout ? QStringList() : original.activities;
        }

        if (role == Qt::UserRole) {
            return m_layoutsTable[row].activities;
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

void Layouts::setOriginalActivitiesForLayout(const Latte::Data::Layout &layout)
{
    if (o_layoutsTable.containsId(layout.id) && m_layoutsTable.containsId(layout.id)) {
        o_layoutsTable[layout.id].activities = layout.activities;

        int mrow = rowForId(layout.id);
        setActivities(mrow, layout.activities);
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

    m_layoutsTable[row].activities = activities;
    emit dataChanged(index(row, BACKGROUNDCOLUMN), index(row,ACTIVITYCOLUMN), roles);
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
}

bool Layouts::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const int row = index.row();
    const int column = index.column();

    if (!m_layoutsTable.rowExists(row) || column<0 || column > ACTIVITYCOLUMN) {
        return false;
    }

    QVector<int> roles;
    roles << role;

    //! common roles for all row cells
    if (role == ISLOCKEDROLE) {
        m_layoutsTable[row].isLocked = value.toBool();
        emit dataChanged(this->index(row,0), this->index(row, ACTIVITYCOLUMN), roles);
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

        if (m_corona->layoutsManager()->synchronizer()->layout(m_layoutsTable[i].name)) {
            iActive = true;
        }

        if (m_layoutsTable[i].isActive != iActive) {
            m_layoutsTable[i].isActive = iActive;

            emit dataChanged(index(i, BACKGROUNDCOLUMN), index(i,ACTIVITYCOLUMN), roles);
        }
    }
}

int Layouts::rowForId(const QString &id) const
{
    return m_layoutsTable.indexOf(id);
}

const Latte::Data::Layout &Layouts::at(const int &row)
{
    return m_layoutsTable[row];
}

const Latte::Data::Layout &Layouts::currentData(const QString &id)
{
    return m_layoutsTable[id];
}


const Latte::Data::Layout Layouts::originalData(const QString &id)
{
    if (o_layoutsTable.containsId(id)){
        return o_layoutsTable[id];
    }

    return Latte::Data::Layout();
}

const Latte::Data::LayoutsTable &Layouts::originalLayoutsData()
{
    return o_layoutsTable;
}

const Latte::Data::LayoutsTable &Layouts::currentLayoutsData()
{
    return m_layoutsTable;
}

void Layouts::setOriginalInMultipleMode(const bool &inmultiple)
{
    setInMultipleMode(inmultiple);

    if (o_inMultipleMode == inmultiple) {
        return;
    }

    o_inMultipleMode = inmultiple;
    emit inMultipleModeChanged();
}

void Layouts::setOriginalData(Latte::Data::LayoutsTable &data)
{
    clear();

    beginInsertRows(QModelIndex(), 0, data.rowCount() - 1);
    o_layoutsTable = data;
    m_layoutsTable = data;
    endInsertRows();

    emit rowsInserted();
}

QList<Latte::Data::Layout> Layouts::alteredLayouts() const
{
    QList<Latte::Data::Layout> layouts;

    for(int i=0; i<rowCount(); ++i) {
        QString currentId = m_layoutsTable[i].id;

        if ((!o_layoutsTable.containsId(currentId))
                || m_layoutsTable[currentId] != o_layoutsTable[currentId]) {
            layouts << m_layoutsTable[i];
        }
    }

    return layouts;
}

//! Activities code
void Layouts::initActivities()
{
    Latte::Data::Activity allActivities;
    allActivities.id = Latte::Data::Layout::ALLACTIVITIESID;
    allActivities.name = QString("[ " + i18n("All Activities") + " ]");
    allActivities.icon = "favorites";
    allActivities.state = KActivities::Info::Stopped;
    m_activitiesTable << allActivities;

    Latte::Data::Activity freeActivities;
    freeActivities.id = Latte::Data::Layout::FREEACTIVITIESID;
    freeActivities.name = QString("[ " + i18n("Free Activities") + " ]");
    freeActivities.icon = "favorites";
    freeActivities.state = KActivities::Info::Stopped;
    m_activitiesTable << freeActivities;

    Latte::Data::Activity currentActivity;
    currentActivity.id = Latte::Data::Layout::CURRENTACTIVITYID;
    currentActivity.name = QString("[ " + i18n("Current Activity") + " ]");
    currentActivity.icon = "favorites";
    currentActivity.state = KActivities::Info::Stopped;
    m_activitiesTable << currentActivity;

    QStringList activities = m_corona->layoutsManager()->synchronizer()->activities();;

    for(const auto &id: activities) {
        KActivities::Info info(id);

        if (info.state() != KActivities::Info::Invalid) {
            onActivityAdded(id);
        }
    }

    connect(m_corona->activitiesConsumer(), &KActivities::Consumer::activityAdded, this, &Layouts::onActivityAdded);
    connect(m_corona->activitiesConsumer(), &KActivities::Consumer::activityRemoved, this, &Layouts::onActivityRemoved);
    connect(m_corona->activitiesConsumer(), &KActivities::Consumer::runningActivitiesChanged, this, &Layouts::onRunningActivitiesChanged);

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

void Layouts::onActivityAdded(const QString &id)
{
    m_activitiesInfo[id] = new KActivities::Info(id, this);

    Latte::Data::Activity activity;
    activity.id = m_activitiesInfo[id]->id();
    activity.name = m_activitiesInfo[id]->name();
    activity.icon = m_activitiesInfo[id]->icon();
    activity.state = m_activitiesInfo[id]->state();
    activity.isCurrent = m_activitiesInfo[id]->isCurrent();

    if (!m_activitiesTable.containsId(id)) {
        m_activitiesTable << activity;
    } else {
        m_activitiesTable[id] = activity;
    }

    connect(m_activitiesInfo[id], &KActivities::Info::nameChanged, [this, id]() {
        onActivityChanged(id);
    });

    connect(m_activitiesInfo[id], &KActivities::Info::iconChanged, [this, id]() {
        onActivityChanged(id);
    });

    connect(m_activitiesInfo[id], &KActivities::Info::isCurrentChanged, [this, id]() {
        onActivityChanged(id);
    });
}

void Layouts::onActivityRemoved(const QString &id)
{
    if (m_activitiesTable.containsId(id)) {
        m_activitiesTable.remove(id);
    }

    if (m_activitiesInfo.contains(id)) {
        KActivities::Info *info = m_activitiesInfo.take(id);
        info->deleteLater();
    }

    activitiesStatesChanged();
}

void Layouts::onActivityChanged(const QString &id)
{
    if (m_activitiesTable.containsId(id) && m_activitiesInfo.contains(id)) {
        m_activitiesTable[id].name = m_activitiesInfo[id]->name();
        m_activitiesTable[id].icon = m_activitiesInfo[id]->icon();
        m_activitiesTable[id].state = m_activitiesInfo[id]->state();
        m_activitiesTable[id].isCurrent = m_activitiesInfo[id]->isCurrent();

        activitiesStatesChanged();
    }
}

void Layouts::onRunningActivitiesChanged(const QStringList &runningIds)
{
    for (int i = 0; i < m_activitiesTable.rowCount(); ++i) {
       if (runningIds.contains(m_activitiesTable[i].id)) {
           m_activitiesTable[i].state = KActivities::Info::Running;
       } else {
           m_activitiesTable[i].state = KActivities::Info::Stopped;
       }
    }

    emit activitiesStatesChanged();
}

}
}
}
