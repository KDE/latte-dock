/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef DOCKCORONA_H
#define DOCKCORONA_H

#include "dockview.h"
#include "globalsettings.h"
#include "../liblattedock/dock.h"

#include <QObject>

#include <KAboutApplicationDialog>
#include <KDeclarative/QmlObject>

namespace Plasma {
class Corona;
class Containment;
class Types;
}

class ScreenPool;
class GlobalSettings;

namespace KActivities {
class Consumer;
}

namespace Latte {

class DockCorona : public Plasma::Corona {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.LatteDock")

public:
    DockCorona(QObject *parent = nullptr);
    virtual ~DockCorona();

    int numScreens() const override;
    QRect screenGeometry(int id) const override;
    QRegion availableScreenRegion(int id) const override;
    QRect availableScreenRect(int id) const override;

    QList<Plasma::Types::Location> freeEdges(int screen) const;
    QList<Plasma::Types::Location> freeEdges(QScreen *screen) const;

    int docksCount(int screen) const;
    int docksCount() const;
    int noDocksWithTasks() const;
    int screenForContainment(const Plasma::Containment *containment) const override;

    void addDock(Plasma::Containment *containment);
    void recreateDock(Plasma::Containment *containment);

    Dock::SessionType currentSession();
    void setCurrentSession(Dock::SessionType session);
    void switchToSession(Dock::SessionType session);

    void aboutApplication();
    void closeApplication();

    ScreenPool *screenPool() const;
    GlobalSettings *globalSettings() const;

public slots:
    void activateLauncherMenu();
    void loadDefaultLayout() override;
    void dockContainmentDestroyed(QObject *cont);

signals:
    void configurationShown(PlasmaQuick::ConfigView *configView);
    void currentSessionChanged(Dock::SessionType type);
    void docksCountChanged();
    void dockLocationChanged();
    void raiseDocksTemporaryChanged();

private slots:
    void destroyedChanged(bool destroyed);
    void showAlternativesForApplet(Plasma::Applet *applet);
    void alternativesVisibilityChanged(bool visible);
    void load();

    void addOutput(QScreen *screen);
    void primaryOutputChanged();
    void screenRemoved(QScreen *screen);
    void screenCountChanged();
    void syncDockViews();

private:
    void cleanConfig();
    void qmlRegisterTypes() const;
    bool appletExists(uint containmentId, uint appletId) const;
    bool containmentContainsTasks(Plasma::Containment *cont);
    bool containmentExists(uint id) const;
    bool heuresticForLoadingDockWithTasks();
    int noDocksForSession(Dock::SessionType session);
    int primaryScreenId() const;

    bool m_activitiesStarting{true};
    //! used to initialize the docks when changing sessions
    bool m_waitingSessionDocksCreation{false};
    //! this is used to check if a dock with tasks in it will be loaded on startup
    bool m_tasksWillBeLoaded{false};
    //! this is used to record the first dock having tasks in it. It is used
    //! to specify which dock will be loaded on startup if a case that no "dock
    //! with tasks" will be loaded otherwise. Currently the older one dock wins
    int m_firstContainmentWithTasks{ -1};

    Dock::SessionType m_session{Dock::DefaultSession};

    QHash<const Plasma::Containment *, DockView *> m_dockViews;
    QHash<const Plasma::Containment *, DockView *> m_waitingDockViews;
    QList<KDeclarative::QmlObject *> m_alternativesObjects;

    QTimer m_docksScreenSyncTimer;

    KActivities::Consumer *m_activityConsumer;
    QPointer<KAboutApplicationDialog> aboutDialog;

    ScreenPool *m_screenPool;
    GlobalSettings *m_globalSettings;
};

}

#endif // DOCKCORONA_H
