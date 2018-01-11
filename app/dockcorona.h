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
#include "globalshortcuts.h"
#include "layoutmanager.h"
#include "universalsettings.h"
#include "../liblattedock/dock.h"
#include "launcherssignals.h"

#include <QObject>

#include <KAboutApplicationDialog>
#include <KDeclarative/QmlObject>

class ScreenPool;
class GlobalShortcuts;
class UniversalSettings;
class LayoutManager;
class LaunchersSignals;

namespace KActivities {
class Consumer;
}

namespace Plasma {
class Corona;
class Containment;
class Types;
}

namespace KWayland {
namespace Client {
class PlasmaShell;
}
}

namespace Latte {

class DockCorona : public Plasma::Corona {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.LatteDock")

public:
    DockCorona(bool defaultLayoutOnStartup = false, QString layoutNameOnStartUp = QString(), QObject *parent = nullptr);
    virtual ~DockCorona();

    int numScreens() const override;
    QRect screenGeometry(int id) const override;
    QRegion availableScreenRegion(int id) const override;
    QRect availableScreenRect(int id) const override;

    //! This is a very generic function in order to return the availableScreenRect of specific screen
    //! by calculating only the user specified visibility modes and edges. Empty QLists for both
    //! arguments mean that all choices are accepted in calculations
    QRect availableScreenRectWithCriteria(int id,
                                          QList<Dock::Visibility> modes = QList<Dock::Visibility>(),
                                          QList<Plasma::Types::Location> edges = QList<Plasma::Types::Location>()) const;

    QList<Plasma::Types::Location> freeEdges(int screen) const;
    QList<Plasma::Types::Location> freeEdges(QScreen *screen) const;

    void loadLatteLayout(QString layoutPath);

    int docksCount() const;
    int docksCount(int screen) const;
    int docksCount(QScreen *screen) const;

    int noDocksWithTasks() const;
    int screenForContainment(const Plasma::Containment *containment) const override;

    void recreateDock(Plasma::Containment *containment);

    void aboutApplication();
    void closeApplication();

    ScreenPool *screenPool() const;
    UniversalSettings *universalSettings() const;
    LayoutManager *layoutManager() const;

    KWayland::Client::PlasmaShell *waylandDockCoronaInterface() const;

public slots:
    void activateLauncherMenu();
    void loadDefaultLayout() override;
    void updateDockItemBadge(QString identifier, QString value);
    void unload();

signals:
    void configurationShown(PlasmaQuick::ConfigView *configView);
    void docksCountChanged();
    void dockLocationChanged();
    void raiseDocksTemporaryChanged();

private slots:
    void showAlternativesForApplet(Plasma::Applet *applet);
    void alternativesVisibilityChanged(bool visible);
    void load();

    void addOutput(QScreen *screen);
    void primaryOutputChanged();
    void screenRemoved(QScreen *screen);
    void screenCountChanged();
    void syncDockViewsToScreens();

private:
    void cleanConfig();
    void qmlRegisterTypes() const;
    void setupWaylandIntegration();

    bool appletExists(uint containmentId, uint appletId) const;
    bool containmentExists(uint id) const;
    bool explicitDockOccupyEdge(int screen, Plasma::Types::Location location)const;
    bool heuresticForLoadingDockWithTasks();
    int noOfDocks();
    int primaryScreenId() const;

    QStringList containmentsIds();
    QStringList appletsIds();

    bool m_activitiesStarting{true};
    //! this is used to enforce loading the default layout on startup
    bool m_defaultLayoutOnStartup{false};
    //! this is used to check if a dock with tasks in it will be loaded on startup
    bool m_tasksWillBeLoaded{false};
    //! this is used to record the first dock having tasks in it. It is used
    //! to specify which dock will be loaded on startup if a case that no "dock
    //! with tasks" will be loaded otherwise. Currently the older one dock wins
    int m_firstContainmentWithTasks{ -1};

    QString m_layoutNameOnStartUp;

    QList<KDeclarative::QmlObject *> m_alternativesObjects;

    QTimer m_docksScreenSyncTimer;

    KActivities::Consumer *m_activityConsumer;
    QPointer<KAboutApplicationDialog> aboutDialog;

    ScreenPool *m_screenPool{nullptr};
    GlobalShortcuts *m_globalShortcuts{nullptr};
    UniversalSettings *m_universalSettings{nullptr};
    LayoutManager *m_layoutManager{nullptr};

    KWayland::Client::PlasmaShell *m_waylandDockCorona{nullptr};

    friend class GlobalShortcuts;
    friend class Layout;
    friend class LayoutManager;
    friend class LaunchersSignals;
};

}

#endif // DOCKCORONA_H
