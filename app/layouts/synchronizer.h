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

// Qt
#include <QObject>
#include <QHash>
#include <QTimer>


namespace Latte {
class CentralLayout;
class SharedLayout;
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

//! This is a Shares map in the following structure:
//! SHARED LAYOUT NAME -> CENTRAL LAYOUT NAMES acting as SHARES
typedef QHash<const QString, QStringList> SharesMap;


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

    void loadLayouts();
    void unloadLayouts();

    void hideAllViews();
    void pauseLayout(QString layoutName);
    void syncActiveLayoutsToOriginalFiles();
    void syncLatteViewsToScreens();
    void syncMultipleLayoutsToActivities(QString layoutForFreeActivities = QString());
    void syncActiveShares(SharesMap &sharesMap, QStringList &deprecatedShares);

    bool latteViewExists(Latte::View *view) const;
    bool layoutExists(QString layoutName) const;
    bool mapHasRecord(const QString &record, SharesMap &map);
    bool registerAtSharedLayout(CentralLayout *central, QString id);
    //! switch to specified layout, default previousMemoryUsage means that it didn't change
    bool switchToLayout(QString layoutName, int previousMemoryUsage = -1);

    int centralLayoutPos(QString id) const;

    QString currentLayoutName() const;
    QString currentLayoutNameInMultiEnvironment() const;
    QString shouldSwitchToLayout(QString activityId);

    QStringList centralLayoutsNames();
    QStringList layouts() const;
    QStringList menuLayouts() const;
    void setMenuLayouts(QStringList layouts);
    QStringList sharedLayoutsNames();
    QStringList storedSharedLayouts() const;

    QStringList activities();
    QStringList runningActivities();
    QStringList freeActivities(); //! These are activities that haven't been assigned to specific layout

    Latte::View *viewForContainment(Plasma::Containment *containment);

    CentralLayout *currentLayout() const;
    CentralLayout *centralLayout(QString id) const;
    SharedLayout *sharedLayout(QString id) const;
    Layout::GenericLayout *layout(QString id) const;

signals:
    void centralLayoutsChanged();
    void currentLayoutNameChanged();
    void layoutsChanged();
    void menuLayoutsChanged();
    void runningActicitiesChanged();

    void currentLayoutIsSwitching(QString layoutName);

private slots:
    void confirmDynamicSwitch();
    void updateDynamicSwitchInterval();
    void updateCurrentLayoutNameInMultiEnvironment();

    void currentActivityChanged(const QString &id);

private:
    void clearSharedLayoutsFromCentralLists();

    void addLayout(CentralLayout *layout);
    void unloadCentralLayout(CentralLayout *layout);
    void unloadSharedLayout(SharedLayout *layout);

    bool layoutIsAssigned(QString layoutName);

    QString layoutPath(QString layoutName);

    QStringList validActivities(QStringList currentList);

private:
    bool m_multipleModeInitialized{false};

    QString m_currentLayoutNameInMultiEnvironment;
    QString m_shouldSwitchToLayout;

    QStringList m_layouts;
    QStringList m_menuLayouts;
    QStringList m_sharedLayoutIds;

    QHash<const QString, QString> m_assignedLayouts;

    QTimer m_dynamicSwitchTimer;

    QList<CentralLayout *> m_centralLayouts;
    QList<SharedLayout *> m_sharedLayouts;

    Layouts::Manager *m_manager;
    KActivities::Controller *m_activitiesController;
};

}
}


#endif
