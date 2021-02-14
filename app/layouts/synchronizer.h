/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LAYOUTSSYNCHRONIZER_H
#define LAYOUTSSYNCHRONIZER_H

// local
#include "../apptypes.h"
#include "../data/layoutdata.h"
#include "../data/layoutstable.h"

// Qt
#include <QObject>
#include <QHash>
#include <QTimer>


namespace Latte {
class CentralLayout;
class View;
namespace Layout{
class GenericLayout;
}
namespace Layouts {
class Manager;
}
}

namespace Plasma {
class Containment;
}

namespace KActivities {
class Controller;
}

namespace Latte {
namespace Layouts {

//! This is a Layouts map in the following structure:
//! ACTIVITY ID -> Layout Names for that activity
typedef QHash<QString, QStringList> AssignedLayoutsHash;

//! Layouts::Synchronizer is a very IMPORTANT class which is responsible
//! for all ACTIVE layouts, meaning layouts that have been loaded
//! in memory.
//!
//! The main task of Synchronizer is to load/unload layouts based
//! on "user preferences"/"activities settings"/"application current
//! phase" (e.g. startup/closing)
//!

class Synchronizer : public QObject {
    Q_OBJECT

public:
    Synchronizer(QObject *parent);
    ~Synchronizer() override;

    void unloadLayouts();

    void hideAllViews();
    void pauseLayout(QString layoutName);
    void syncActiveLayoutsToOriginalFiles();
    void syncLatteViewsToScreens();
    void syncMultipleLayoutsToActivities();

    //! In that case single layout file must be removed after loading the new layout
    void setIsSingleLayoutInDeprecatedRenaming(const bool &enabled);

    bool latteViewExists(Latte::View *view) const;
    bool layoutExists(QString layoutName) const;
    //! switch to specified layout, default previousMemoryUsage means that it didn't change
    bool switchToLayout(QString layoutName,  MemoryUsage::LayoutsMemory newMemoryUsage = MemoryUsage::Current);

    int centralLayoutPos(QString id) const;

    QStringList centralLayoutsNames();
    QStringList currentLayoutsNames() const;
    QStringList layouts() const;
    QStringList menuLayouts() const;

    QStringList activities();
    QStringList freeActivities();
    QStringList runningActivities();
    QStringList freeRunningActivities(); //! These are activities that haven't been assigned to specific layout
    QStringList validActivities(const QStringList &layoutActivities);

    Latte::View *viewForContainment(Plasma::Containment *containment);
    Latte::View *viewForContainment(uint id);

    QList<CentralLayout *> currentLayouts() const;
    QList<Latte::View *> currentViews() const;
    QList<Latte::View *> currentViewsWithPlasmaShortcuts() const;
    QList<Latte::View *> sortedCurrentViews() const;
    QList<Latte::View *> viewsBasedOnActivityId(const QString &id) const;

    CentralLayout *centralLayout(QString layoutname) const;
    Layout::GenericLayout *layout(QString layoutname) const;

    QList<CentralLayout *> centralLayoutsForActivity(const QString activityid) const;

    KActivities::Controller *activitiesController() const;

    Data::LayoutsTable layoutsTable() const;
    void setLayoutsTable(const Data::LayoutsTable &table);

public slots:
    void initLayouts();
    void updateKWinDisabledBorders();

    void updateLayoutsTable();

signals:
    void centralLayoutsChanged();
    void layoutsChanged();
    void runningActicitiesChanged();

    void currentLayoutIsSwitching(QString layoutName);

    void newLayoutAdded(const Data::Layout &layout);
    void layoutActivitiesChanged(const Data::Layout &layout);

private slots:
    void onActivityRemoved(const QString &activityid);
    void onCurrentActivityChanged(const QString &activityid);
    void onLayoutAdded(const QString &layoutpath);

    void reloadAssignedLayouts();

private:
    void addLayout(CentralLayout *layout);
    void unloadCentralLayout(CentralLayout *layout);
    void unloadLayouts(const QStringList &layoutNames);

    bool initSingleMode(QString layoutName);
    bool initMultipleMode(QString layoutName);

    bool switchToLayoutInMultipleMode(QString layoutName);
    bool switchToLayoutInSingleMode(QString layoutName);
    bool switchToLayoutInMultipleModeBasedOnActivities(const QString &layoutName);

    bool isAssigned(QString layoutName) const;
    bool memoryInitialized() const;

    QString layoutPath(QString layoutName);

private:
    bool m_multipleModeInitialized{false};
    bool m_isLoaded{false};
    bool m_isSingleLayoutInDeprecatedRenaming{false};

    Data::LayoutsTable m_layouts;
    QList<CentralLayout *> m_centralLayouts;
    AssignedLayoutsHash m_assignedLayouts;

    Layouts::Manager *m_manager;
    KActivities::Controller *m_activitiesController;
};

}
}


#endif
