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
#include "../../lattecorona.h"
#include "../../data/activitydata.h"
#include "../../data/layoutdata.h"
#include "../../data/layouticondata.h"
#include "../../data/layoutstable.h"

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
        ACTIVITYCOLUMN
    };

    enum LayoutUserRoles
    {
        IDROLE = Qt::UserRole + 1,
        ISACTIVEROLE,
        ISLOCKEDROLE,
        INMULTIPLELAYOUTSROLE,
        BACKGROUNDUSERROLE,
        ASSIGNEDACTIVITIESROLE,
        ALLACTIVITIESSORTEDROLE,
        ALLACTIVITIESDATAROLE,
        ALLLAYOUTSROLE,
        SORTINGROLE,
        ISNEWLAYOUTROLE,
        LAYOUTHASCHANGESROLE,
        ORIGINALISSHOWNINMENUROLE,
        ORIGINALHASBORDERSROLE,
        ORIGINALASSIGNEDACTIVITIESROLE,
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
    bool layoutsAreChanged() const;
    bool modeIsChanged() const;

    bool inMultipleMode() const;
    void setInMultipleMode(bool inMultiple);

    int rowCount() const;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    const Latte::Data::Layout &at(const int &row);
    const Latte::Data::Layout &currentData(const QString &id);
    const Latte::Data::Layout originalData(const QString &id);

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    int rowForId(const QString &id) const;

    void clear();
    //! all current data will become also original
    void applyData();
    //! all original data will become also current
    void resetData();

    void appendOriginalLayout(const Latte::Data::Layout &layout);
    void appendLayout(const Latte::Data::Layout &layout);
    void removeLayout(const QString &id);
    void setLayoutProperties(const Latte::Data::Layout &layout);

    QString layoutNameForFreeActivities() const;
    void setCurrentLayoutForFreeActivities(const QString &id);
    void setOriginalLayoutForFreeActivities(const QString &id);

    void setIconsPath(QString iconsPath);

    QList<Latte::Data::Layout> alteredLayouts() const;

    const Latte::Data::LayoutsTable &currentLayoutsData();
    const Latte::Data::LayoutsTable &originalLayoutsData();

    void setOriginalInMultipleMode(const bool &inmultiple);
    void setOriginalData(Latte::Data::LayoutsTable &data);

    void setOriginalActivitiesForLayout(const Latte::Data::Layout &layout);

signals:
    void inMultipleModeChanged();
    void nameDuplicated(const QString &provenId, const QString &trialId);
    void rowsInserted();

private slots:
    void updateActiveStates();

    void activitiesStatesChanged();
    void onActivityAdded(const QString &id);
    void onActivityRemoved(const QString &id);
    void onActivityChanged(const QString &id);
    void onRunningActivitiesChanged(const QStringList &runningIds);

private:
    void initActivities();

    void setActivities(const int &row, const QStringList &activities);
    void setId(const int &row, const QString &newId);

    QString sortingPriority(const SortingPriority &priority, const int &row) const;
    QString sortableText(const int &priority, const int &row) const;

    QStringList cleanStrings(const QStringList &original, const QStringList &occupied);

    QList<Latte::Data::LayoutIcon> icons(const int &row) const;
    QList<Latte::Data::LayoutIcon> iconsForCentralLayout(const int &row) const;

private:
    QString m_iconsPath;

    Latte::Data::ActivitiesTable m_activitiesTable;
    QHash<QString, KActivities::Info *> m_activitiesInfo;

    //! original data
    bool o_inMultipleMode{false};
    Latte::Data::LayoutsTable o_layoutsTable;

    //! current data
    bool m_inMultipleMode{false};
    Latte::Data::LayoutsTable m_layoutsTable;

    Latte::Corona *m_corona{nullptr};
};

}
}
}

#endif
