/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LATTECORONA_H
#define LATTECORONA_H

// local
#include <coretypes.h>
#include "plasma/quick/configview.h"
#include "layouts/storage.h"
#include "view/panelshadows_p.h"

// Qt
#include <QObject>
#include <QTimer>

// Plasma
#include <Plasma/Corona>

// KDE
#include <KAboutApplicationDialog>

namespace KDeclarative {
class QmlObjectSharedEngine;
}

namespace Plasma {
class Corona;
class Containment;
class Types;
}

namespace PlasmaQuick {
class ConfigView;
}

namespace KActivities {
class Consumer;
}

namespace KWayland {
namespace Client {
class PlasmaShell;
}
}

namespace Latte {
class CentralLayout;
class ScreenPool;
class GlobalShortcuts;
class UniversalSettings;
class View;
class ViewSettingsFactory;
namespace Indicator{
class Factory;
}
namespace Layout{
class GenericLayout;
}
namespace Layouts{
class Manager;
}
namespace PlasmaExtended{
class ScreenGeometries;
class ScreenPool;
class Theme;
}
namespace Templates {
class Manager;
}
namespace WindowSystem{
class AbstractWindowInterface;
}
}

namespace Latte {

class Corona : public Plasma::Corona
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.LatteDock")

public:
    Corona(bool defaultLayoutOnStartup = false,
               QString layoutNameOnStartUp = QString(),
               QString addViewTemplateName = QString(),
               int userSetMemoryUsage = -1,
               QObject *parent = nullptr);
    virtual ~Corona();

    bool inQuit() const;

    int numScreens() const override;
    QRect screenGeometry(int id) const override;
    QRegion availableScreenRegion(int id) const override;
    QRect availableScreenRect(int id) const override;

    //! This is a very generic function in order to return the availableScreenRect of specific screen
    //! by calculating only the user specified visibility modes and edges. Empty QLists for both
    //! arguments mean that all choices are accepted in calculations. ignoreExternalPanels means that
    //! external panels should be not considered in the calculations
    QRect availableScreenRectWithCriteria(int id,
                                          QString activityid = QString(),
                                          QList<Types::Visibility> ignoreModes = QList<Types::Visibility>(),
                                          QList<Plasma::Types::Location> ignoreEdges = QList<Plasma::Types::Location>(),
                                          bool ignoreExternalPanels = true,
                                          bool desktopUse = false) const;

    QRegion availableScreenRegionWithCriteria(int id,
                                              QString activityid = QString(),
                                              QList<Types::Visibility> ignoreModes = QList<Types::Visibility>(),
                                              QList<Plasma::Types::Location> ignoreEdges = QList<Plasma::Types::Location>(),
                                              bool ignoreExternalPanels = true,
                                              bool desktopUse = false) const;

    int screenForContainment(const Plasma::Containment *containment) const override;

    KWayland::Client::PlasmaShell *waylandCoronaInterface() const;

    KActivities::Consumer *activitiesConsumer() const;
    GlobalShortcuts *globalShortcuts() const;
    ScreenPool *screenPool() const;
    UniversalSettings *universalSettings() const;
    ViewSettingsFactory *viewSettingsFactory() const;
    Layouts::Manager *layoutsManager() const;   
    Templates::Manager *templatesManager() const;

    Indicator::Factory *indicatorFactory() const;

    PlasmaExtended::ScreenPool *plasmaScreenPool() const;
    PlasmaExtended::Theme *themeExtended() const;

    WindowSystem::AbstractWindowInterface *wm() const;

    PanelShadows *dialogShadows() const;

    //! Needs to be called in order to import and load application properly after application
    //! finished all its exit operations
    void importFullConfiguration(const QString &file);

    //! these functions are used from context menu through containmentactions    
    void quitApplication();
    void switchToLayout(QString layout);
    void importLayoutFile(const QString &filepath, const QString &suggestedLayoutName = QString());
    void showSettingsWindow(int page);

    QStringList contextMenuData(const uint &containmentId);
    QStringList viewTemplatesData();

public slots:
    void aboutApplication();
    void activateLauncherMenu();
    void loadDefaultLayout() override;

    void setAutostart(const bool &enabled);

    void addView(const uint &containmentId, const QString &templateId);
    void duplicateView(const uint &containmentId);
    void exportViewTemplate(const uint &containmentId);
    void moveViewToLayout(const uint &containmentId, const QString &layoutName);
    void removeView(const uint &containmentId);

    void setBackgroundFromBroadcast(QString activity, QString screenName, QString filename);
    void setBroadcastedBackgroundsEnabled(QString activity, QString screenName, bool enabled);
    void showAlternativesForApplet(Plasma::Applet *applet);
    void toggleHiddenState(QString layoutName, QString viewName, QString screenName, int screenEdge);

    //! values are separated with a "-" character
    void windowColorScheme(QString windowIdAndScheme);
    void updateDockItemBadge(QString identifier, QString value);

    void unload();

signals:
    void configurationShown(PlasmaQuick::ConfigView *configView);
    void viewLocationChanged();
    void raiseViewsTemporaryChanged();
    void availableScreenRectChangedFrom(Latte::View *origin);
    void availableScreenRegionChangedFrom(Latte::View *origin);
    void verticalUnityViewHasFocus();

private slots:
    void alternativesVisibilityChanged(bool visible);
    void load();

    void onAboutToQuit();

    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);
    void onScreenCountChanged();
    void onScreenGeometryChanged(const QRect &geometry);
    void syncLatteViewsToScreens();

private:
    void cleanConfig();
    void qmlRegisterTypes() const;
    void setupWaylandIntegration();

    bool appletExists(uint containmentId, uint appletId) const;
    bool containmentExists(uint id) const;

    int primaryScreenId() const;

    QStringList containmentsIds();
    QStringList appletsIds();

    Layout::GenericLayout *layout(QString name) const;
    CentralLayout *centralLayout(QString name) const;

private:

    bool m_activitiesStarting{true};
    bool m_defaultLayoutOnStartup{false}; //! this is used to enforce loading the default layout on startup
    bool m_inStartup{true}; //! this is used in order to identify when application is still in startup phase
    bool m_inQuit{false}; //! this is used in order to identify when application is in quit phase
    bool m_quitTimedEnded{false}; //! this is used on destructor in order to delay it and slide-out the views

    //!it can be used on startup to change memory usage from command line
    int m_userSetMemoryUsage{ -1};

    QString m_layoutNameOnStartUp;
    QString m_startupAddViewTemplateName;
    QString m_importFullConfigurationFile;

    QList<KDeclarative::QmlObjectSharedEngine *> m_alternativesObjects;

    QTimer m_viewsScreenSyncTimer;

    KActivities::Consumer *m_activitiesConsumer;
    QPointer<KAboutApplicationDialog> aboutDialog;

    ScreenPool *m_screenPool{nullptr};
    UniversalSettings *m_universalSettings{nullptr};
    ViewSettingsFactory *m_viewSettingsFactory{nullptr};
    GlobalShortcuts *m_globalShortcuts{nullptr};

    Indicator::Factory *m_indicatorFactory{nullptr};
    Layouts::Manager *m_layoutsManager{nullptr};
    Templates::Manager *m_templatesManager{nullptr};

    PlasmaExtended::ScreenGeometries *m_plasmaGeometries{nullptr};
    PlasmaExtended::ScreenPool *m_plasmaScreenPool{nullptr};
    PlasmaExtended::Theme *m_themeExtended{nullptr};

    WindowSystem::AbstractWindowInterface *m_wm{nullptr};

    PanelShadows *m_dialogShadows{nullptr};

    KWayland::Client::PlasmaShell *m_waylandCorona{nullptr};

    friend class GlobalShortcuts;
    friend class Layouts::Manager;
    friend class Layouts::Storage;
};

}

#endif // LATTECORONA_H
