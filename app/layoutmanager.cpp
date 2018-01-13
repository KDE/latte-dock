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

#include "layoutmanager.h"
#include "infoview.h"
#include "screenpool.h"

#include <QDir>
#include <QFile>
#include <QQmlProperty>
#include <QtDBus/QtDBus>

#include <KActivities/Consumer>
#include <KLocalizedString>
#include <KNotification>

namespace Latte {

const int MultipleLayoutsPresetId = 10;

LayoutManager::LayoutManager(QObject *parent)
    : QObject(parent),
      m_importer(new Importer(this)),
      m_launchersSignals(new LaunchersSignals(this))
{
    m_corona = qobject_cast<DockCorona *>(parent);

    if (m_corona) {
        //! create the add widgets action
        const QIcon addWidIcon = QIcon::fromTheme("add");
        m_addWidgetsAction = new QAction(addWidIcon, i18n("Add Widgets..."), this);
        m_addWidgetsAction->setStatusTip(i18n("Show Plasma Widget Explorer"));
        connect(m_addWidgetsAction, &QAction::triggered, this, &LayoutManager::showWidgetsExplorer);

        connect(m_corona->universalSettings(), &UniversalSettings::currentLayoutNameChanged, this, &LayoutManager::currentLayoutNameChanged);
        connect(m_corona->universalSettings(), &UniversalSettings::showInfoWindowChanged, this, &LayoutManager::showInfoWindowChanged);

        m_dynamicSwitchTimer.setSingleShot(true);
        showInfoWindowChanged();
        connect(&m_dynamicSwitchTimer, &QTimer::timeout, this, &LayoutManager::confirmDynamicSwitch);

        m_currentActivityId = m_corona->m_activityConsumer->currentActivity();
    }
}

LayoutManager::~LayoutManager()
{
    m_importer->deleteLater();
    m_launchersSignals->deleteLater();

    while (!m_activeLayouts.isEmpty()) {
        Layout *layout = m_activeLayouts.at(0);
        m_activeLayouts.removeFirst();
        layout->unloadContainments();
        layout->unloadDockViews();
        layout->deleteLater();
    }
}

void LayoutManager::load()
{
    int configVer = m_corona->universalSettings()->version();
    qDebug() << "Universal Settings version : " << configVer;

    if (configVer < 2 && QFile(QDir::homePath() + "/.config/lattedockrc").exists()) {
        qDebug() << "Latte must update its configuration...";
        m_importer->updateOldConfiguration();
        importPresets(false);
    } else if (!QFile(QDir::homePath() + "/.config/lattedockrc").exists()) {
        //startup create what is necessary....
        QDir layoutDir(QDir::homePath() + "/.config/latte");

        if (!layoutDir.exists()) {
            QDir(QDir::homePath() + "/.config").mkdir("latte");
        }

        newLayout(i18n("My Layout"));
        importPresets(false);
        m_corona->universalSettings()->setCurrentLayoutName(i18n("My Layout"));
        m_corona->universalSettings()->setVersion(2);
    }

    //! Check if the multiple-layouts hidden file is present, add it if it isnt
    if (!QFile(QDir::homePath() + "/.config/latte/" + Layout::MultipleLayoutsName + ".layout.latte").exists()) {
        importPreset(MultipleLayoutsPresetId, false);
    }

    qDebug() << "Latte is loading  its layouts...";

    connect(m_corona->m_activityConsumer, &KActivities::Consumer::currentActivityChanged,
            this, &LayoutManager::currentActivityChanged);

    loadLayouts();
}

void LayoutManager::unload()
{
    foreach (auto layout, m_activeLayouts) {
        layout->unloadContainments();
        layout->unloadDockViews();
        layout->deleteLater();
    }

    if (memoryUsage() == Dock::MultipleLayouts) {
        auto containmentsEntries = m_corona->config()->group("Containments");
        containmentsEntries.deleteGroup();
        containmentsEntries.sync();
    }

    QString temp1File = QDir::homePath() + "/.config/lattedock.copy1.bak";
    QString temp2File = QDir::homePath() + "/.config/lattedock.copy2.bak";

    QFile file1(temp1File);
    QFile file2(temp2File);

    if (file1.exists())
        file1.remove();

    if (file2.exists())
        file2.remove();
}

DockCorona *LayoutManager::corona()
{
    return m_corona;
}

Importer *LayoutManager::importer()
{
    return m_importer;
}

QAction *LayoutManager::addWidgetsAction()
{
    return m_addWidgetsAction;
}

LaunchersSignals *LayoutManager::launchersSignals()
{
    return m_launchersSignals;
}

QString LayoutManager::currentLayoutName() const
{
    if (memoryUsage() == Dock::SingleLayout) {
        return m_corona->universalSettings()->currentLayoutName();
    } else if (memoryUsage() == Dock::MultipleLayouts) {
        foreach (auto layout, m_activeLayouts) {
            if (layout->activities().contains(m_currentActivityId)) {
                return layout->name();
            }
        }

        foreach (auto layout, m_activeLayouts) {
            if ((layout->name() != Layout::MultipleLayoutsName) && (layout->activities().isEmpty())) {
                return layout->name();
            }
        }
    }

    return QString();
}

QString LayoutManager::defaultLayoutName() const
{
    QByteArray presetNameOrig = QString("preset" + QString::number(1)).toUtf8();
    QString presetPath = m_corona->kPackage().filePath(presetNameOrig);
    QString presetName = Layout::layoutName(presetPath);
    QByteArray presetNameChars = presetName.toUtf8();
    presetName = i18n(presetNameChars);

    return presetName;
}

bool LayoutManager::layoutExists(QString layoutName) const
{
    return m_layouts.contains(layoutName);
}

QStringList LayoutManager::layouts() const
{
    return m_layouts;
}

QStringList LayoutManager::menuLayouts() const
{
    QStringList fixedMenuLayouts = m_menuLayouts;

    //! in case the current layout isnt checked to be shown in the menus
    //! we must add it on top
    if (!fixedMenuLayouts.contains(currentLayoutName())) {
        fixedMenuLayouts.prepend(currentLayoutName());
    }

    return fixedMenuLayouts;
}

void LayoutManager::setMenuLayouts(QStringList layouts)
{
    if (m_menuLayouts == layouts) {
        return;
    }

    m_menuLayouts = layouts;
    emit menuLayoutsChanged();
}

QStringList LayoutManager::activities()
{
    return m_corona->m_activityConsumer->activities();
}

QStringList LayoutManager::runningActivities()
{
    return m_corona->m_activityConsumer->runningActivities();
}

QStringList LayoutManager::orphanedActivities()
{
    QStringList orphans;

    foreach (auto activity, activities()) {
        if (m_assignedLayouts[activity].isEmpty()) {
            orphans.append(activity);
        }
    }

    return orphans;
}

QStringList LayoutManager::presetsPaths() const
{
    return m_presetsPaths;
}

QString LayoutManager::layoutPath(QString layoutName)
{
    QString path = QDir::homePath() + "/.config/latte/" + layoutName + ".layout.latte";

    if (!QFile(path).exists()) {
        path = "";
    }

    return path;
}

Dock::LayoutsMemoryUsage LayoutManager::memoryUsage() const
{
    return m_corona->universalSettings()->layoutsMemoryUsage();
}

void LayoutManager::setMemoryUsage(Dock::LayoutsMemoryUsage memoryUsage)
{
    m_corona->universalSettings()->setLayoutsMemoryUsage(memoryUsage);
}

void LayoutManager::addDock(Plasma::Containment *containment, bool forceLoading, int expDockScreen)
{
    if (memoryUsage() == Dock::SingleLayout) {
        m_activeLayouts.at(0)->addDock(containment, forceLoading, expDockScreen);
    } else if (memoryUsage() == Dock::MultipleLayouts) {
        QString layoutId = containment->config().readEntry("layoutId", QString());

        if (!layoutId.isEmpty()) {
            auto layout = activeLayout(layoutId);

            if (layout) {
                layout->addDock(containment, forceLoading, expDockScreen);
            }
        }
    }
}

void LayoutManager::recreateDock(Plasma::Containment *containment)
{
    if (memoryUsage() == Dock::SingleLayout) {
        m_activeLayouts.at(0)->recreateDock(containment);
    } else if (memoryUsage() == Dock::MultipleLayouts) {
        QString layoutId = containment->config().readEntry("layoutId", QString());

        if (!layoutId.isEmpty()) {
            auto layout = activeLayout(layoutId);

            if (layout) {
                layout->recreateDock(containment);
            }
        }
    }
}

void LayoutManager::syncDockViewsToScreens()
{
    foreach (auto layout, m_activeLayouts) {
        layout->syncDockViewsToScreens();
    }
}

QHash<const Plasma::Containment *, DockView *> *LayoutManager::currentDockViews() const
{
    if (memoryUsage() == Dock::SingleLayout) {
        return m_activeLayouts.at(0)->dockViews();
    } else {
        foreach (auto layout, m_activeLayouts) {
            if (layout->activities().contains(m_currentActivityId)) {
                return layout->dockViews();
            }
        }

        foreach (auto layout, m_activeLayouts) {
            if ((layout->name() != Layout::MultipleLayoutsName) && (layout->activities().isEmpty())) {
                return layout->dockViews();
            }
        }
    }
}

QHash<const Plasma::Containment *, DockView *> *LayoutManager::layoutDockViews(const QString &layoutName) const
{
    Layout *layout = activeLayout(layoutName);

    if (layout) {
        return layout->dockViews();
    }

    return nullptr;
}

Layout *LayoutManager::activeLayout(QString id) const
{
    for (int i = 0; i < m_activeLayouts.size(); ++i) {
        Layout *layout = m_activeLayouts.at(i);

        if (layout->name() == id) {
            return layout;
        }
    }

    return nullptr;
}

void LayoutManager::currentActivityChanged(const QString &id)
{
    m_currentActivityId = id;

    if (memoryUsage() == Dock::SingleLayout) {
        qDebug() << "activity changed :: " << id;

        m_shouldSwitchToLayout = shouldSwitchToLayout(id);

        m_dynamicSwitchTimer.start();
    }
}

void LayoutManager::showInfoWindowChanged()
{
    if (m_corona->universalSettings()->showInfoWindow()) {
        m_dynamicSwitchTimer.setInterval(1800);
    } else {
        m_dynamicSwitchTimer.setInterval(2300);
    }
}

QString LayoutManager::shouldSwitchToLayout(QString activityId)
{
    if (m_assignedLayouts.contains(activityId) && m_assignedLayouts[activityId] != currentLayoutName()) {
        return m_assignedLayouts[activityId];
    } else if (!m_assignedLayouts.contains(activityId) && !m_corona->universalSettings()->lastNonAssignedLayoutName().isEmpty()
               && m_corona->universalSettings()->lastNonAssignedLayoutName() != currentLayoutName()) {
        return m_corona->universalSettings()->lastNonAssignedLayoutName();
    }

    return QString();
}

void LayoutManager::confirmDynamicSwitch()
{
    QString tempShouldSwitch = shouldSwitchToLayout(m_corona->m_activityConsumer->currentActivity());

    if (tempShouldSwitch.isEmpty()) {
        return;
    }

    if (m_shouldSwitchToLayout == tempShouldSwitch && m_shouldSwitchToLayout != currentLayoutName()) {
        qDebug() << "dynamic switch to layout :: " << m_shouldSwitchToLayout;

        emit currentLayoutIsChanging();

        if (m_corona->universalSettings()->showInfoWindow()) {
            showInfoWindow(i18n("Switching to layout <b>%0</b> ...").arg(m_shouldSwitchToLayout), 4000);
        }

        QTimer::singleShot(500, [this, tempShouldSwitch]() {
            switchToLayout(tempShouldSwitch);
        });
    } else {
        m_shouldSwitchToLayout = tempShouldSwitch;
        m_dynamicSwitchTimer.start();
    }
}

void LayoutManager::loadLayouts()
{
    m_layouts.clear();
    m_menuLayouts.clear();
    m_presetsPaths.clear();
    m_assignedLayouts.clear();

    QDir layoutDir(QDir::homePath() + "/.config/latte");
    QStringList filter;
    filter.append(QString("*.layout.latte"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    foreach (auto layout, files) {
        Layout layoutSets(this, layoutDir.absolutePath() + "/" + layout);

        QStringList validActivityIds = validActivities(layoutSets.activities());
        layoutSets.setActivities(validActivityIds);

        foreach (auto activity, validActivityIds) {
            m_assignedLayouts[activity] = layoutSets.name();
        }

        m_layouts.append(layoutSets.name());

        if (layoutSets.showInMenu()) {
            m_menuLayouts.append(layoutSets.name());
        }
    }

    m_presetsPaths.append(m_corona->kPackage().filePath("preset1"));
    m_presetsPaths.append(m_corona->kPackage().filePath("preset2"));
    m_presetsPaths.append(m_corona->kPackage().filePath("preset3"));
    m_presetsPaths.append(m_corona->kPackage().filePath("preset4"));

    emit layoutsChanged();
    emit menuLayoutsChanged();
}

void LayoutManager::loadLatteLayout(QString layoutPath)
{
    qDebug() << " -------------------------------------------------------------------- ";
    qDebug() << " -------------------------------------------------------------------- ";

    if (m_corona->containments().size() > 0) {
        qDebug() << "LOAD LATTE LAYOUT ::: There are still containments present !!!! :: " << m_corona->containments().size();
    }

    if (!layoutPath.isEmpty() && m_corona->containments().size() == 0) {
        qDebug() << "LOADING CORONA LAYOUT:" << layoutPath;
        m_corona->loadLayout(layoutPath);

        //! ~~~ ADDING DOCKVIEWS AND ENFORCE LOADING IF TASKS ARENT PRESENT BASED ON SCREENS ~~~ !//

        //! this is used to record the first dock having tasks in it. It is used
        //! to specify which dock will be loaded on startup if a case that no "dock
        //! with tasks" will be loaded otherwise. Currently the older one dock wins
        int firstContainmentWithTasks = -1;

        //! this is used to check if a dock with tasks in it will be loaded on startup
        bool tasksWillBeLoaded =  heuresticForLoadingDockWithTasks(&firstContainmentWithTasks);

        qDebug() << "TASKS WILL BE PRESENT AFTER LOADING ::: " << tasksWillBeLoaded;

        foreach (auto containment, m_corona->containments()) {
            //! forceDockLoading is used when a latte configuration based on the
            //! current running screens does not provide a dock containing tasks.
            //! in such case the lowest latte containment containing tasks is loaded
            //! and it forcefully becomes primary dock
            if (!tasksWillBeLoaded && firstContainmentWithTasks == containment->id()) {
                tasksWillBeLoaded = true; //this protects by loading more than one dock at startup
                addDock(containment, true);
            } else {
                addDock(containment);
            }
        }
    }
}

void LayoutManager::importLatteLayout(QString layoutPath)
{
//! This might not be needed as it is Layout responsibility
}

bool LayoutManager::switchToLayout(QString layoutName)
{
    if (m_currentLayout && m_currentLayout->name() == layoutName) {
        return false;
    }

    QString lPath = layoutPath(layoutName);

    if (lPath.isEmpty() && layoutName == i18n("Alternative")) {
        lPath = newLayout(i18n("Alternative"), i18n("Default"));
    }

    if (!lPath.isEmpty()) {
        emit currentLayoutIsChanging();

        //! this code must be called asynchronously because it is called
        //! also from qml (Tasks plasmoid). This change fixes a very important
        //! crash when switching sessions through the Tasks plasmoid Context menu
        //! Latte was unstable and was crashing very often during changing
        //! sessions.
        QTimer::singleShot(250, [this, layoutName, lPath]() {
            qDebug() << layoutName << " - " << lPath;
            QString fixedLPath = lPath;
            QString fixedLayoutName = layoutName;

            bool initializingMultipleLayouts{false};

            if (memoryUsage() == Dock::MultipleLayouts && !activeLayout(Layout::MultipleLayoutsName)) {
                initializingMultipleLayouts = true;
            }

            if (memoryUsage() == Dock::SingleLayout || initializingMultipleLayouts) {
                while (!m_activeLayouts.isEmpty()) {
                    Layout *layout = m_activeLayouts.at(0);
                    m_activeLayouts.removeFirst();
                    layout->unloadContainments();
                    layout->unloadDockViews();
                    delete layout;
                }

                if (initializingMultipleLayouts) {
                    fixedLayoutName = QString(Layout::MultipleLayoutsName);
                    fixedLPath = layoutPath(fixedLayoutName);
                }

                Layout *newLayout = new Layout(this, fixedLPath, fixedLayoutName);
                m_activeLayouts.append(newLayout);
                newLayout->initToCorona(m_corona);

                loadLatteLayout(fixedLPath);

                emit activeLayoutsChanged();
            }

            if (memoryUsage() == Dock::MultipleLayouts)  {
                syncMultipleLayoutsToActivities(layoutName);
            }

            m_corona->universalSettings()->setCurrentLayoutName(layoutName);

            if (!layoutIsAssigned(layoutName)) {
                m_corona->universalSettings()->setLastNonAssignedLayoutName(layoutName);
            }
        });
    }

    return true;
}

void LayoutManager::syncMultipleLayoutsToActivities(QString layoutForOrphans)
{
    QList<Layout *> layoutsToUnload;
    QStringList layoutsToLoad;

    bool allRunningActivitiesWillBeReserved{true};

    if (layoutForOrphans.isEmpty() || m_assignedLayouts.values().contains(layoutForOrphans)) {
        layoutForOrphans = m_corona->universalSettings()->lastNonAssignedLayoutName();
    }

    foreach (auto activity, runningActivities()) {
        if (!m_assignedLayouts[activity].isEmpty()) {
            if (!layoutsToLoad.contains(m_assignedLayouts[activity])) {
                layoutsToLoad.append(m_assignedLayouts[activity]);
            }
        } else {
            allRunningActivitiesWillBeReserved = false;
        }
    }

    foreach (auto layout, m_activeLayouts) {
        Layout *tempLayout{nullptr};

        if (!layoutsToLoad.contains(layout->name())) {
            tempLayout = layout;
        } else if (layout->activities().isEmpty() && allRunningActivitiesWillBeReserved) {
            //! in such case the layout for the orphaned must be unloaded
            tempLayout = layout;
        }

        if (tempLayout && !layoutsToUnload.contains(tempLayout)) {
            layoutsToUnload << tempLayout;
        }
    }

    //! Unload no needed Layouts
    foreach (auto layout, layoutsToUnload) {
        if (layout->name() != Layout::MultipleLayoutsName) {
            Layout *tempLayout = layoutsToUnload.at(0);
            layoutsToUnload.removeFirst();
            tempLayout->unloadContainments();
            tempLayout->unloadDockViews();
            delete tempLayout;
        }
    }

    //! Add Layout for orphan activities
    if (!allRunningActivitiesWillBeReserved) {
        if (!activeLayout(layoutForOrphans)) {
            Layout *newLayout = new Layout(this, layoutPath(layoutForOrphans), layoutForOrphans);

            if (newLayout) {
                m_activeLayouts.append(newLayout);
                newLayout->initToCorona(m_corona);
                newLayout->importToCorona();
            }
        }
    }

    //! Add needed Layouts based on Activities
    foreach (auto layoutName, layoutsToLoad) {
        if (!activeLayout(layoutName)) {
            Layout *newLayout = new Layout(this, QString(layoutPath(layoutName)), layoutName);

            if (newLayout) {
                m_activeLayouts.append(newLayout);
                newLayout->initToCorona(m_corona);
                newLayout->importToCorona();
            }
        }
    }

    emit activeLayoutsChanged();
}

QString LayoutManager::newLayout(QString layoutName, QString preset)
{
    QDir layoutDir(QDir::homePath() + "/.config/latte");
    QStringList filter;
    filter.append(QString(layoutName + "*.layout.latte"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    //! if the newLayout already exists provide a newName that doesnt
    if (files.count() >= 1) {
        int newCounter = files.count() + 1;

        layoutName = layoutName + "-" + QString::number(newCounter);
    }

    QString newLayoutPath = layoutDir.absolutePath() + "/" + layoutName + ".layout.latte";

    qDebug() << "adding layout : " << layoutName << " based on preset:" << preset;

    if (preset == i18n("Default") && !QFile(newLayoutPath).exists()) {
        qDebug() << "adding layout : succeed";
        QFile(m_corona->kPackage().filePath("preset1")).copy(newLayoutPath);
    }

    return newLayoutPath;
}

//! This function figures in the beginning if a dock with tasks
//! in it will be loaded taking into account also the screens are present.
bool LayoutManager::heuresticForLoadingDockWithTasks(int *firstContainmentWithTasks)
{
    foreach (auto containment, m_corona->containments()) {
        QString plugin = containment->pluginMetaData().pluginId();

        if (plugin == "org.kde.latte.containment") {
            bool onPrimary = containment->config().readEntry("onPrimary", true);
            int lastScreen =  containment->lastScreen();

            qDebug() << "containment values: " << onPrimary << " - " << lastScreen;


            bool containsTasks = false;

            foreach (auto applet, containment->applets()) {
                const auto &provides = KPluginMetaData::readStringList(applet->pluginMetaData().rawData(), QStringLiteral("X-Plasma-Provides"));

                if (provides.contains(QLatin1String("org.kde.plasma.multitasking"))) {
                    containsTasks = true;
                    break;
                }
            }

            if (containsTasks) {
                *firstContainmentWithTasks = containment->id();

                if (onPrimary) {
                    return true;
                } else {
                    if (lastScreen >= 0) {
                        QString connector = m_corona->screenPool()->connector(lastScreen);

                        foreach (auto scr, qGuiApp->screens()) {
                            if (scr && scr->name() == connector) {
                                return true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}


void LayoutManager::importDefaultLayout(bool newInstanceIfPresent)
{
    importPreset(1, newInstanceIfPresent);

    if (newInstanceIfPresent) {
        loadLayouts();
    }
}

void LayoutManager::importPresets(bool includeDefault)
{
    int start = 1;

    if (!includeDefault) {
        start = 2;
    }

    for (int i = start; i <= 4; ++i) {
        importPreset(i, false);
    }
}

void LayoutManager::importPreset(int presetNo, bool newInstanceIfPresent)
{
    QByteArray presetNameOrig = QString("preset" + QString::number(presetNo)).toUtf8();
    QString presetPath = m_corona->kPackage().filePath(presetNameOrig);
    QString presetName = Layout::layoutName(presetPath);
    QByteArray presetNameChars = presetName.toUtf8();
    presetName = i18n(presetNameChars);

    //! hide the multiple layouts layout file from user
    presetName = (presetNo == MultipleLayoutsPresetId) ? "." + presetName : presetName;

    QString newLayoutFile = "";

    if (newInstanceIfPresent) {
        newLayoutFile = QDir::homePath() + "/.config/latte/" + m_importer->uniqueLayoutName(presetName) + ".layout.latte";
    } else {
        newLayoutFile = QDir::homePath() + "/.config/latte/" + presetName + ".layout.latte";
    }

    if (!QFile(newLayoutFile).exists()) {
        QFile(presetPath).copy(newLayoutFile);
    }
}

QStringList LayoutManager::validActivities(QStringList currentList)
{
    QStringList validIds;

    foreach (auto activity, currentList) {
        if (activities().contains(activity)) {
            validIds.append(activity);
        }
    }

    return validIds;
}

bool LayoutManager::layoutIsAssigned(QString layoutName)
{
    QHashIterator<const QString, QString> i(m_assignedLayouts);

    while (i.hasNext()) {
        i.next();

        if (i.value() == layoutName) {
            return true;
        }
    }

    return false;
}

void LayoutManager::showLayoutConfigDialog()
{
    if (!m_layoutConfigDialog)
        m_layoutConfigDialog = new LayoutConfigDialog(nullptr, this);

    m_layoutConfigDialog->show();

    if (m_layoutConfigDialog->isMinimized()) {
        m_layoutConfigDialog->showNormal();
    }

    m_layoutConfigDialog->activateWindow();
}

void LayoutManager::showInfoWindow(QString info, int duration)
{
    foreach (auto screen, qGuiApp->screens()) {
        InfoView *infoView = new InfoView(m_corona, info, screen);

        infoView->show();

        QTimer::singleShot(duration, [this, infoView]() {
            infoView->deleteLater();
        });
    }
}

void LayoutManager::showWidgetsExplorer()
{
    QDBusInterface iface("org.kde.plasmashell", "/PlasmaShell", "", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.call("toggleWidgetExplorer");
    }
}

//! it is used just in order to provide translations for the presets
void LayoutManager::ghostForTranslatedPresets()
{
    QString preset1 = i18n("Default");
    QString preset2 = i18n("Plasma");
    QString preset3 = i18n("Unity");
    QString preset4 = i18n("Extended");
}

}
