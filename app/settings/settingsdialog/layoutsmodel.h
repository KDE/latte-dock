/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
        ISCONSIDEREDACTIVEROLE,
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
        ERRORSROLE,
        WARNINGSROLE
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

    bool hasChangedData() const;
    bool layoutsAreChanged() const;
    bool modeIsChanged() const;

    bool inMultipleMode() const;
    void setInMultipleMode(bool inMultiple);

    bool hasEnabledLayout() const;
    bool hasEnabledLayoutInAllActitivities() const;
    bool hasEnabledLayoutInFreeActivities() const;
    bool hasEnabledLayoutInCurrentActivity() const;

    int rowCount() const;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QString currentActivityId() const;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    const Latte::Data::Layout &at(const int &row);
    const Latte::Data::Layout &currentData(const QString &id);
    const Latte::Data::Layout originalData(const QString &id);

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    int rowForId(const QString &id) const;

    const Latte::Data::LayoutIcon currentLayoutIcon(const QString &id) const;

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

    QList<Latte::Data::Layout> alteredLayouts() const;

    const Latte::Data::LayoutsTable &currentLayoutsData();
    const Latte::Data::LayoutsTable &originalLayoutsData();

    void setOriginalInMultipleMode(const bool &inmultiple);
    void setOriginalData(Latte::Data::LayoutsTable &data);

    void setOriginalActivitiesForLayout(const Latte::Data::Layout &layout);
    void setOriginalViewsForLayout(const Latte::Data::Layout &layout);

signals:
    void activitiesStatesChanged();
    void inMultipleModeChanged();
    void nameDuplicated(const QString &provenId, const QString &trialId);
    void rowsInserted();

private slots:
    void updateActiveStates();
    void updateConsideredActiveStates();

    void onActivitiesStatesChanged();
    void onActivityAdded(const QString &id);
    void onActivityRemoved(const QString &id);
    void onActivityChanged(const QString &id);
    void onRunningActivitiesChanged(const QStringList &runningIds);

private:
    void initActivities();

    void setActivities(const int &row, const QStringList &activities);
    void setId(const int &row, const QString &newId);

    bool containsSpecificRunningActivity(const QStringList &runningIds, const Latte::Data::Layout &layout) const;

    QString sortingPriority(const SortingPriority &priority, const int &row) const;
    QString sortableText(const int &priority, const int &row) const;

    QStringList cleanStrings(const QStringList &original, const QStringList &occupied);

    Latte::Data::LayoutIcon icon(const int &row) const;

private:
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
