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

#ifndef SETTINGSLAYOUTSMODEL_H
#define SETTINGSLAYOUTSMODEL_H

// local
#include "../data/activitydata.h"
#include "../data/layoutdata.h"
#include "../data/layouticondata.h"
#include "../data/layoutstable.h"
#include "../../lattecorona.h"

// Qt
#include <QAbstractTableModel>
#include <QModelIndex>


namespace Latte {
namespace Settings {
namespace Model {

class Layouts : public QAbstractTableModel
{
    Q_OBJECT

public:

    enum Columns
    {
        IDCOLUMN = 0,
        HIDDENTEXTCOLUMN,
        BACKGROUNDCOLUMN,
        NAMECOLUMN,
        MENUCOLUMN,
        BORDERSCOLUMN,
        ACTIVITYCOLUMN,
        SHAREDCOLUMN
    };

    enum LayoutUserRoles
    {
        IDROLE = Qt::UserRole + 1,
        ISACTIVEROLE,
        ISLOCKEDROLE,
        ISSHAREDROLE,
        INMULTIPLELAYOUTSROLE,
        ASSIGNEDACTIVITIESROLE,
        ASSIGNEDACTIVITIESFROMSHAREDROLE,
        ALLACTIVITIESSORTEDROLE,
        ALLACTIVITIESDATAROLE,
        ALLLAYOUTSROLE,
        SHAREDTOINEDITROLE,
        SORTINGROLE,
        ISNEWLAYOUTROLE,
        LAYOUTHASCHANGESROLE,
        ORIGINALISSHOWNINMENUROLE,
        ORIGINALHASBORDERSROLE,
        ORIGINALASSIGNEDACTIVITIESROLE,
        ORIGINALSHARESROLE
    };

    enum SortingPriority
    {
        NORMALPRIORITY = 8000,
        MEDIUMPRIORITY = 6000,
        HIGHPRIORITY = 4000,
        HIGHESTPRIORITY = 2000
    };

    explicit Layouts(QObject *parent, Latte::Corona *corona);
    ~Layouts();

    bool containsCurrentName(const QString &name) const;

    bool dataAreChanged() const;

    bool inMultipleMode() const;
    void setInMultipleMode(bool inMultiple);

    int rowCount() const;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    const Data::Layout &at(const int &row);
    const Data::Layout &currentData(const QString &id);
    const Data::Layout originalData(const QString &id);

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    int rowForId(const QString &id) const;

    void clear();
    //! all current data will become also original
    void applyData();
    //! all original data will become also current
    void resetData();

    void appendLayout(const Settings::Data::Layout &layout);
    void removeLayout(const QString &id);

    QString layoutNameForFreeActivities() const;
    void setCurrentLayoutForFreeActivities(const QString &id);
    void setOriginalLayoutForFreeActivities(const QString &id);

    void setIconsPath(QString iconsPath);

    bool shareIsAvailable(const QString id) const;
    QStringList availableShareIdsFor(const QString id) const;

    QList<Data::Layout> alteredLayouts() const;

    const Data::LayoutsTable &currentLayoutsData();
    const Data::LayoutsTable &originalLayoutsData();
    void setOriginalData(Data::LayoutsTable &data, const bool &inmultiple);

signals:
    void inMultipleModeChanged();
    void nameDuplicated(const QString &provenId, const QString &trialId);
    void rowsInserted();

private slots:
    void updateActiveStates();

    void activitiesStatesChanged();
    void on_activityAdded(const QString &id);
    void on_activityRemoved(const QString &id);
    void on_activityChanged(const QString &id);
    void on_runningActivitiesChanged(const QStringList &runningIds);

private:
    void initActivities();

    void assignFreeActivitiesLayoutAt(const QString &layoutName);
    void autoAssignFreeActivitiesLayout();

    void setActivities(const int &row, const QStringList &activities);
    void setId(const int &row, const QString &newId);
    void setShares(const int &row, const QStringList &shares);

    QString sortingPriority(const SortingPriority &priority, const int &row) const;
    QString sortableText(const int &priority, const int &row) const;

    QStringList cleanStrings(const QStringList &original, const QStringList &occupied);

    QStringList assignedActivitiesFromShared(const int &row) const;

    QList<Data::LayoutIcon> icons(const int &row) const;

private:
    //! break MVC only when a SharedTo editor is created
    //! because we want to move the dot indicator in the Activities delegate
    //! when that happens
    int m_sharedToInEditRow;

    QString m_iconsPath;

    Data::ActivitiesMap m_activitiesMap;
    QHash<QString, KActivities::Info *> m_activitiesInfo;

    //! original data
    bool o_inMultipleMode{false};
    Settings::Data::LayoutsTable o_layoutsTable;

    //! current data
    bool m_inMultipleMode{false};
    Data::LayoutsTable m_layoutsTable;

    Latte::Corona *m_corona{nullptr};
};

}
}
}

#endif
