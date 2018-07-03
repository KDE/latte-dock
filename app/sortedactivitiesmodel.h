/*
 *   Copyright (C) 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SORTED_ACTIVITY_MODEL
#define SORTED_ACTIVITY_MODEL

// Qt
#include <QSortFilterProxyModel>

#include <Plasma>

// KDE
#include <KActivities/Consumer>
#include <KActivities/Info>
#include <KActivities/ActivitiesModel>

typedef QHash<Plasma::Types::Location, float> EdgesHash;

class SortedActivitiesModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool inhibitUpdates READ inhibitUpdates WRITE setInhibitUpdates NOTIFY inhibitUpdatesChanged)

public:
    SortedActivitiesModel(const QVector<KActivities::Info::State> &states, QObject *parent = nullptr);
    ~SortedActivitiesModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    float luminasFromFile(QString imageFile, int edge);
    QString relativeActivity(int relative) const;

protected:
    /*enum AdditionalRoles {
        LastTimeUsed       = KActivities::ActivitiesModel::UserRole,
        LastTimeUsedString = KActivities::ActivitiesModel::UserRole + 1,
        WindowCount        = KActivities::ActivitiesModel::UserRole + 2,
        HasWindows         = KActivities::ActivitiesModel::UserRole + 3
    };*/

public Q_SLOTS:
    bool inhibitUpdates() const;
    void setInhibitUpdates(bool sortByLastUsedTime);

    void onBackgroundsUpdated(const QStringList &changedBackgrounds);
    void onCurrentActivityChanged(const QString &currentActivity);

    QString activityIdForRow(int row) const;
    QString activityIdForIndex(const QModelIndex &index) const;
    int rowForActivityId(const QString &activity) const;

    void rowChanged(int row, const QVector<int> &roles);

Q_SIGNALS:
    void inhibitUpdatesChanged(bool inhibitUpdates);

private:
    bool m_inhibitUpdates;

    QString m_previousActivity;

    KActivities::ActivitiesModel *m_activitiesModel = nullptr;
    KActivities::Consumer *m_activities = nullptr;

    QHash<QString, EdgesHash> m_luminasCache;
};

#endif // SORTED_ACTIVITY_MODEL
