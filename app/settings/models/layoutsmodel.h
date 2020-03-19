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
        LAYOUTISACTIVEROLE = Qt::UserRole + 1,
        LAYOUTISLOCKEDROLE,
        LAYOUTISSHAREDROLE,
        LAYOUTNAMEWASEDITEDROLE,
        INMULTIPLELAYOUTSROLE,
        ASSIGNEDACTIVITIESROLE,
        ALLACTIVITIESSORTEDROLE,
        ALLACTIVITIESDATAROLE,
        ALLLAYOUTSROLE,
        SHAREDTOINEDIT
    };

    explicit Layouts(QObject *parent, Latte::Corona *corona);
    ~Layouts();

    bool containsCurrentName(const QString &name) const;

    bool inMultipleMode() const;
    void setInMultipleMode(bool inMultiple);

    QString idForOriginalName(const QString &name);
    QString idForCurrentName(const QString &name);

    int rowCount() const;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    const Data::Layout &at(const int &row);

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    int rowForId(const QString &id) const;

    void clear();
    void applyCurrentNames();
    void appendLayout(const Settings::Data::Layout &layout);
    void removeLayout(const QString &id);

    QString layoutNameForFreeActivities() const;
    void setLayoutNameForFreeActivities(const QString &name);

    QStringList availableShareIdsFor(const QString id) const;

    const Data::LayoutsTable &currentData();
    void setCurrentData(Data::LayoutsTable &data);

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

    QStringList cleanStrings(const QStringList &original, const QStringList &occupied);

private:
    bool m_inMultipleMode{false};
    Data::LayoutsTable m_layoutsTable;

    int m_sharedToInEditRow{-1};

    Data::ActivitiesMap m_activitiesMap;
    QHash<QString, KActivities::Info *> m_activitiesInfo;

    Latte::Corona *m_corona{nullptr};
};

}
}
}

#endif
