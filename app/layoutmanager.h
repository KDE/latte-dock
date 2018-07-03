/*
*  Copyright 2017  Smith AR <audoban@openmailbox.org>
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

#ifndef LAYOUTMANAGER_H
#define LAYOUTMANAGER_H

#include "launcherssignals.h"
#include "settingsdialog.h"

#include <QAction>
#include <QObject>
#include <QPointer>

#include <KLocalizedString>

namespace Plasma {
class Containment;
class Types;
}

namespace KActivities {
class Controller;
}

namespace Latte {
class DockCorona;
class DockView;
class Importer;
class Layout;
class LaunchersSignals;
}

namespace Latte {

//! This class is responsible to manipulate all layouts.
//! add,remove,rename, update configurations etc.
class LayoutManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentLayoutName READ currentLayoutName NOTIFY currentLayoutNameChanged)

    Q_PROPERTY(QStringList layouts READ layouts NOTIFY layoutsChanged)
    Q_PROPERTY(QStringList menuLayouts READ menuLayouts NOTIFY menuLayoutsChanged)

    Q_PROPERTY(LaunchersSignals *launchersSignals READ launchersSignals NOTIFY launchersSignalsChanged)

public:
    LayoutManager(QObject *parent = nullptr);
    ~LayoutManager() override;

    DockCorona *corona();
    Importer *importer();

    void load();
    void loadLayoutOnStartup(QString layoutName);
    void unload();
    void addDock(Plasma::Containment *containment, bool forceLoading = false, int expDockScreen = -1);
    void hideAllDocks();
    void pauseLayout(QString layoutName);
    void syncDockViewsToScreens();
    void syncActiveLayoutsToOriginalFiles();

    bool layoutExists(QString layoutName) const;

    QString shouldSwitchToLayout(QString activityId);

    QString currentLayoutName() const;
    QString defaultLayoutName() const;

    QStringList layouts() const;
    QStringList menuLayouts() const;
    QStringList presetsPaths() const;

    Dock::LayoutsMemoryUsage memoryUsage() const;
    void setMemoryUsage(Dock::LayoutsMemoryUsage memoryUsage);

    QHash<const Plasma::Containment *, DockView *> *currentDockViews() const;
    QHash<const Plasma::Containment *, DockView *> *layoutDockViews(const QString &layoutName) const;
    //! returns an active layout with that #id (name), it returns null if such
    //! layout cant be found
    Layout *activeLayout(QString id) const;
    int activeLayoutPos(QString id) const;

    LaunchersSignals *launchersSignals();

    QStringList activities();
    QStringList runningActivities();
    QStringList orphanedActivities(); //! These are activities that havent been assigned to specific layout

    void importDefaultLayout(bool newInstanceIfPresent = false);
    void importPresets(bool includeDefault = false);

public slots:
    void showAboutDialog();
    void updateColorizerSupport();

    void hideLatteSettingsDialog();
    Q_INVOKABLE void showLatteSettingsDialog(int page = Latte::Dock::LayoutPage);

    //! switch to specified layout, default previousMemoryUsage means that it didnt change
    Q_INVOKABLE bool switchToLayout(QString layoutName, int previousMemoryUsage = -1);

    Q_INVOKABLE int layoutsMemoryUsage();

    //! creates a new layout with layoutName based on the preset
    Q_INVOKABLE QString newLayout(QString layoutName, QString preset = i18n("Default"));

    Q_INVOKABLE QStringList activeLayoutsNames();

signals:
    void activeLayoutsChanged();
    void currentLayoutChanged();
    void currentLayoutNameChanged();
    void launchersSignalsChanged();
    void layoutsChanged();
    void menuLayoutsChanged();

    void currentLayoutIsSwitching(QString layoutName);

private slots:
    void currentActivityChanged(const QString &id);
    void showInfoWindowChanged();
    void syncMultipleLayoutsToActivities(QString layoutForOrphans = QString());

private:
    void cleanupOnStartup(QString path); //!remove deprecated or oldstyle config options
    void clearUnloadedContainmentsFromLinkedFile(QStringList containmentsIds, bool bypassChecks = false);
    void confirmDynamicSwitch();
    //! it is used just in order to provide translations for the presets
    void ghostForTranslatedPresets();
    //! This function figures in the beginning if a dock with tasks
    //! in it will be loaded taking into account also the screens are present.
    //! returns true if it will be loaded, false otherwise
    //! firstContainmentWithTasks = the first containment containing a taskmanager plasmoid
    bool heuresticForLoadingDockWithTasks(int *firstContainmentWithTasks);
    void importLatteLayout(QString layoutPath);
    void importPreset(int presetNo, bool newInstanceIfPresent = false);
    void loadLatteLayout(QString layoutPath);
    void loadLayouts();
    void setMenuLayouts(QStringList layouts);
    void showInfoWindow(QString info, int duration, QStringList activities = {"0"});
    void updateCurrentLayoutNameInMultiEnvironment();

    bool layoutIsAssigned(QString layoutName);

    QString layoutPath(QString layoutName);

    QStringList validActivities(QStringList currentList);

private:
    QString m_currentLayoutNameInMultiEnvironment;
    QString m_shouldSwitchToLayout;

    QStringList m_layouts;
    QStringList m_menuLayouts;
    QStringList m_presetsPaths;

    QHash<const QString, QString> m_assignedLayouts;

    QTimer m_dynamicSwitchTimer;

    QPointer<Latte::SettingsDialog> m_latteSettingsDialog;

    DockCorona *m_corona{nullptr};
    Importer *m_importer{nullptr};
    LaunchersSignals *m_launchersSignals{nullptr};

    QList<Layout *> m_activeLayouts;

    KActivities::Controller *m_activitiesController;


    friend class SettingsDialog;
};

}

#endif // LAYOUTMANAGER_H
