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

#include "synchronizer.h"

//! local
#include "importer.h"
#include "manager.h"
#include "../lattecorona.h"
#include "../layout/centrallayout.h"
#include "../layout/genericlayout.h"
#include "../layout/sharedlayout.h"
#include "../settings/universalsettings.h"
#include "../view/view.h"

// Qt
#include <QDir>
#include <QFile>

// Plasma
#include <Plasma/Containment>

// KDE
#include <KActivities/Consumer>
#include <KActivities/Controller>

namespace Latte {
namespace Layouts {

Synchronizer::Synchronizer(QObject *parent)
    : QObject(parent),
      m_activitiesController(new KActivities::Controller)
{
    m_manager = qobject_cast<Manager *>(parent);

    //! Dynamic Switching
    m_dynamicSwitchTimer.setSingleShot(true);
    updateDynamicSwitchInterval();
    connect(m_manager->corona()->universalSettings(), &UniversalSettings::showInfoWindowChanged, this, &Synchronizer::updateDynamicSwitchInterval);
    connect(&m_dynamicSwitchTimer, &QTimer::timeout, this, &Synchronizer::confirmDynamicSwitch);

    //! KActivities tracking
    connect(m_manager->corona()->activitiesConsumer(), &KActivities::Consumer::currentActivityChanged,
            this, &Synchronizer::currentActivityChanged);

    connect(m_manager->corona()->activitiesConsumer(), &KActivities::Consumer::runningActivitiesChanged,
            this, [&]() {
        if (m_manager->memoryUsage() == Types::MultipleLayouts) {
            syncMultipleLayoutsToActivities();
        }
    });
}

Synchronizer::~Synchronizer()
{
    m_activitiesController->deleteLater();
}

bool Synchronizer::latteViewExists(Latte::View *view) const
{
    for (const auto layout : m_centralLayouts) {
        for (const auto &v : layout->latteViews()) {
            if (v == view) {
                return true;
            }
        }
    }

    return false;
}

bool Synchronizer::layoutExists(QString layoutName) const
{
    return m_layouts.contains(layoutName);
}


bool Synchronizer::layoutIsAssigned(QString layoutName)
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

bool Synchronizer::mapHasRecord(const QString &record, SharesMap &map)
{
    for (SharesMap::iterator i=map.begin(); i!=map.end(); ++i) {
        if (i.value().contains(record)) {
            return true;
        }
    }

    return false;
}

bool Synchronizer::registerAtSharedLayout(CentralLayout *central, QString id)
{
    if (m_manager->memoryUsage() == Types::SingleLayout || centralLayout(id)) {
        //! if memory is functioning to SINGLE mode OR shared layout has already
        //! been loaded as CentralLayout
        return false;
    }

    for (int i = 0; i < m_sharedLayouts.size(); ++i) {
        SharedLayout *layout = m_sharedLayouts.at(i);

        if (layout->name() == id) {
            layout->addCentralLayout(central);
            return true;
        }
    }

    //! If SharedLayout was not found, we must create it
    SharedLayout *top = new SharedLayout(central, this, Importer::layoutFilePath(id));
    m_sharedLayouts.append(top);
    top->importToCorona();

    connect(top, &SharedLayout::layoutDestroyed, this, &Synchronizer::unloadSharedLayout);

    return true;
}

int Synchronizer::centralLayoutPos(QString id) const
{
    for (int i = 0; i < m_centralLayouts.size(); ++i) {
        CentralLayout *layout = m_centralLayouts.at(i);

        if (layout->name() == id) {

            return i;
        }
    }

    return -1;
}

QString Synchronizer::currentLayoutName() const
{
    if (m_manager->memoryUsage() == Types::SingleLayout) {
        return m_manager->corona()->universalSettings()->currentLayoutName();
    } else if (m_manager->memoryUsage() == Types::MultipleLayouts) {
        return currentLayoutNameInMultiEnvironment();
    }

    return QString();
}

QString Synchronizer::currentLayoutNameInMultiEnvironment() const
{
    return m_currentLayoutNameInMultiEnvironment;
}

QString Synchronizer::layoutPath(QString layoutName)
{
    QString path = QDir::homePath() + "/.config/latte/" + layoutName + ".layout.latte";

    if (!QFile(path).exists()) {
        path = "";
    }

    return path;
}

QStringList Synchronizer::activities()
{
    return m_manager->corona()->activitiesConsumer()->activities();
}

QStringList Synchronizer::runningActivities()
{
    return m_manager->corona()->activitiesConsumer()->runningActivities();
}

QStringList Synchronizer::orphanedActivities()
{
    QStringList orphans;

    for (const auto &activity : activities()) {
        if (m_assignedLayouts[activity].isEmpty()) {
            orphans.append(activity);
        }
    }

    return orphans;
}

QStringList Synchronizer::centralLayoutsNames()
{
    QStringList names;

    if (m_manager->memoryUsage() == Types::SingleLayout) {
        names << currentLayoutName();
    } else {
        for (int i = 0; i < m_centralLayouts.size(); ++i) {
            CentralLayout *layout = m_centralLayouts.at(i);
            names << layout->name();
        }
    }

    return names;
}

QStringList Synchronizer::layouts() const
{
    return m_layouts;
}

QStringList Synchronizer::menuLayouts() const
{
    QStringList fixedMenuLayouts = m_menuLayouts;

    //! in case the current layout isnt checked to be shown in the menus
    //! we must add it on top
    if (!fixedMenuLayouts.contains(currentLayoutName()) && m_manager->memoryUsage() == Types::SingleLayout) {
        fixedMenuLayouts.prepend(currentLayoutName());
    } else if (m_manager->memoryUsage() == Types::MultipleLayouts) {
        for (const auto layout : m_centralLayouts) {
            if (!fixedMenuLayouts.contains(layout->name())) {
                fixedMenuLayouts.prepend(layout->name());
            }
        }
    }

    return fixedMenuLayouts;
}

void Synchronizer::setMenuLayouts(QStringList layouts)
{
    if (m_menuLayouts == layouts) {
        return;
    }

    m_menuLayouts = layouts;
    emit menuLayoutsChanged();
}

QString Synchronizer::shouldSwitchToLayout(QString activityId)
{
    if (m_assignedLayouts.contains(activityId) && m_assignedLayouts[activityId] != currentLayoutName()) {
        return m_assignedLayouts[activityId];
    } else if (!m_assignedLayouts.contains(activityId) && !m_manager->corona()->universalSettings()->lastNonAssignedLayoutName().isEmpty()
               && m_manager->corona()->universalSettings()->lastNonAssignedLayoutName() != currentLayoutName()) {
        return m_manager->corona()->universalSettings()->lastNonAssignedLayoutName();
    }

    return QString();
}

QStringList Synchronizer::sharedLayoutsNames()
{
    QStringList names;

    for (int i = 0; i < m_sharedLayouts.size(); ++i) {
        SharedLayout *layout = m_sharedLayouts.at(i);
        names << layout->name();
    }

    return names;
}

QStringList Synchronizer::storedSharedLayouts() const
{
    return m_sharedLayoutIds;
}

QStringList Synchronizer::validActivities(QStringList currentList)
{
    QStringList validIds;

    for (const auto &activity : currentList) {
        if (activities().contains(activity)) {
            validIds.append(activity);
        }
    }

    return validIds;
}

CentralLayout *Synchronizer::centralLayout(QString id) const
{
    for (int i = 0; i < m_centralLayouts.size(); ++i) {
        CentralLayout *layout = m_centralLayouts.at(i);

        if (layout->name() == id) {

            return layout;
        }
    }

    return nullptr;
}

CentralLayout *Synchronizer::currentLayout() const
{
    if (m_manager->memoryUsage() == Types::SingleLayout) {
        return m_centralLayouts.at(0);
    } else {
        for (auto layout : m_centralLayouts) {
            if (layout->activities().contains(m_manager->corona()->activitiesConsumer()->currentActivity())) {
                return layout;
            }
        }

        for (auto layout : m_centralLayouts) {
            if (layout->activities().isEmpty()) {
                return layout;
            }
        }
    }

    return nullptr;
}

Layout::GenericLayout *Synchronizer::layout(QString id) const
{
    Layout::GenericLayout *l = centralLayout(id);

    if (!l) {
        l = sharedLayout(id);
    }

    return l;
}

SharedLayout *Synchronizer::sharedLayout(QString id) const
{
    for (int i = 0; i < m_sharedLayouts.size(); ++i) {
        SharedLayout *layout = m_sharedLayouts.at(i);

        if (layout->name() == id) {
            return layout;
        }
    }

    return nullptr;
}

Latte::View *Synchronizer::viewForContainment(Plasma::Containment *containment)
{
    for (auto layout : m_centralLayouts) {
        Latte::View *view = layout->viewForContainment(containment);

        if (view) {
            return view;
        }
    }

    for (auto layout : m_sharedLayouts) {
        Latte::View *view = layout->viewForContainment(containment);

        if (view) {
            return view;
        }
    }

    return nullptr;
}

void Synchronizer::addLayout(CentralLayout *layout)
{
    if (!m_centralLayouts.contains(layout)) {
        m_centralLayouts.append(layout);
        layout->initToCorona(m_manager->corona());
    }
}

void Synchronizer::clearSharedLayoutsFromCentralLists()
{
    QStringList unassign;

    for(const QString &name : m_sharedLayoutIds) {
        //! remove from ContextMenu
        m_menuLayouts.removeAll(name);

        //! remove from layouts assigned to activities
        QHashIterator<const QString, QString> i(m_assignedLayouts);

        while (i.hasNext()) {
            i.next();

            if (i.value() == name) {
                unassign << i.key();
            }
        }
    }

    for(const QString &activity : unassign) {
        m_assignedLayouts.remove(activity);
    }
}

void Synchronizer::confirmDynamicSwitch()
{
    QString tempShouldSwitch = shouldSwitchToLayout(m_manager->corona()->activitiesConsumer()->currentActivity());

    if (tempShouldSwitch.isEmpty()) {
        return;
    }

    if (m_shouldSwitchToLayout == tempShouldSwitch && m_shouldSwitchToLayout != currentLayoutName()) {
        qDebug() << "dynamic switch to layout :: " << m_shouldSwitchToLayout;

        emit currentLayoutIsSwitching(currentLayoutName());

        if (m_manager->corona()->universalSettings()->showInfoWindow()) {
            m_manager->showInfoWindow(i18n("Switching to layout <b>%0</b> ...").arg(m_shouldSwitchToLayout), 4000);
        }

        QTimer::singleShot(500, [this, tempShouldSwitch]() {
            switchToLayout(tempShouldSwitch);
        });
    } else {
        m_shouldSwitchToLayout = tempShouldSwitch;
        m_dynamicSwitchTimer.start();
    }
}

void Synchronizer::currentActivityChanged(const QString &id)
{
    if (m_manager->memoryUsage() == Types::SingleLayout) {
        qDebug() << "activity changed :: " << id;

        m_shouldSwitchToLayout = shouldSwitchToLayout(id);

        m_dynamicSwitchTimer.start();
    } else if (m_manager->memoryUsage() == Types::MultipleLayouts) {
        updateCurrentLayoutNameInMultiEnvironment();
    }
}

void Synchronizer::hideAllViews()
{
    for (const auto layout : m_centralLayouts) {
        emit currentLayoutIsSwitching(layout->name());
    }
}

void Synchronizer::pauseLayout(QString layoutName)
{
    if (m_manager->memoryUsage() == Types::MultipleLayouts) {
        CentralLayout *layout = centralLayout(layoutName);

        if (layout && !layout->activities().isEmpty()) {
            int i = 0;

            for (const auto &activityId : layout->activities()) {
                //! Stopping the activities must be done asynchronous because otherwise
                //! the activity manager cant close multiple activities
                QTimer::singleShot(i * 1000, [this, activityId]() {
                    m_activitiesController->stopActivity(activityId);
                });

                i = i + 1;
            }
        }
    }
}

void Synchronizer::syncActiveLayoutsToOriginalFiles()
{
    if (m_manager->memoryUsage() == Types::MultipleLayouts) {
        for (const auto layout : m_centralLayouts) {
            layout->syncToLayoutFile();
        }

        for (const auto layout : m_sharedLayouts) {
            layout->syncToLayoutFile();
        }
    }
}

void Synchronizer::syncLatteViewsToScreens()
{
    for (const auto layout : m_sharedLayouts) {
        layout->syncLatteViewsToScreens();
    }

    for (const auto layout : m_centralLayouts) {
        layout->syncLatteViewsToScreens();
    }
}

void Synchronizer::unloadCentralLayout(CentralLayout *layout)
{
    int pos = m_centralLayouts.indexOf(layout);

    if (pos>=0) {
        CentralLayout *central = m_centralLayouts.takeAt(0);

        if (m_multipleModeInitialized) {
            central->syncToLayoutFile(true);
        }

        central->unloadContainments();
        central->unloadLatteViews();

        if (m_multipleModeInitialized) {
            m_manager->clearUnloadedContainmentsFromLinkedFile(central->unloadedContainmentsIds(), true);
        }

        delete central;
    }
}

void Synchronizer::unloadSharedLayout(SharedLayout *layout)
{
    if (m_sharedLayouts.contains(layout)) {
        disconnect(layout, &SharedLayout::layoutDestroyed, this, &Synchronizer::unloadSharedLayout);
        int pos = m_sharedLayouts.indexOf(layout);
        SharedLayout *shared = m_sharedLayouts.takeAt(pos);
        shared->syncToLayoutFile(true);
        shared->unloadContainments();
        shared->unloadLatteViews();
        m_manager->clearUnloadedContainmentsFromLinkedFile(shared->unloadedContainmentsIds(), true);

        delete layout;
    }
}


void Synchronizer::loadLayouts()
{
    m_layouts.clear();
    m_menuLayouts.clear();
    m_assignedLayouts.clear();
    m_sharedLayoutIds.clear();

    QDir layoutDir(QDir::homePath() + "/.config/latte");
    QStringList filter;
    filter.append(QString("*.layout.latte"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    for (const auto &layout : files) {
        if (layout.contains(Layout::AbstractLayout::MultipleLayoutsName)) {
            //! IMPORTANT: DONT ADD MultipleLayouts hidden file in layouts list
            continue;
        }

        CentralLayout centralLayout(this, layoutDir.absolutePath() + "/" + layout);

        QStringList validActivityIds = validActivities(centralLayout.activities());
        centralLayout.setActivities(validActivityIds);

        for (const auto &activity : validActivityIds) {
            m_assignedLayouts[activity] = centralLayout.name();
        }

        m_layouts.append(centralLayout.name());

        if (centralLayout.showInMenu()) {
            m_menuLayouts.append(centralLayout.name());
        }

        QString sharedName = centralLayout.sharedLayoutName();

        if (!sharedName.isEmpty() && !m_sharedLayoutIds.contains(sharedName)) {
            m_sharedLayoutIds << sharedName;
        }
    }

    //! Shared Layouts should not be used for Activities->Layouts assignments or published lists
    clearSharedLayoutsFromCentralLists();

    emit layoutsChanged();
    emit menuLayoutsChanged();
}


void Synchronizer::unloadLayouts()
{
    //! Unload all CentralLayouts
    while (!m_centralLayouts.isEmpty()) {
        CentralLayout *layout = m_centralLayouts.at(0);
        unloadCentralLayout(layout);
    }

    m_multipleModeInitialized = false;
}

void Synchronizer::updateCurrentLayoutNameInMultiEnvironment()
{
    for (const auto layout : m_centralLayouts) {
        if (layout->activities().contains(m_manager->corona()->activitiesConsumer()->currentActivity())) {
            m_currentLayoutNameInMultiEnvironment = layout->name();
            emit currentLayoutNameChanged();
            return;
        }
    }

    for (const auto layout : m_centralLayouts) {
        if (layout->activities().isEmpty()) {
            m_currentLayoutNameInMultiEnvironment = layout->name();
            emit currentLayoutNameChanged();
            return;
        }
    }
}

void Synchronizer::updateDynamicSwitchInterval()
{
    if (m_manager->corona()->universalSettings()->showInfoWindow()) {
        m_dynamicSwitchTimer.setInterval(1800);
    } else {
        m_dynamicSwitchTimer.setInterval(2300);
    }
}

bool Synchronizer::switchToLayout(QString layoutName, int previousMemoryUsage)
{
    if (m_centralLayouts.size() > 0 && currentLayoutName() == layoutName && previousMemoryUsage == -1) {
        return false;
    }

    //! First Check If that Layout is already present and in that case
    //! we can just switch to the proper Activity
    if (m_manager->memoryUsage() == Types::MultipleLayouts && previousMemoryUsage == -1) {
        CentralLayout *layout = centralLayout(layoutName);

        if (layout) {

            QStringList appliedActivities = layout->appliedActivities();
            QString nextActivity = !layout->lastUsedActivity().isEmpty() ? layout->lastUsedActivity() : appliedActivities[0];

            //! it means we are at a foreign activity
            if (!appliedActivities.contains(m_manager->corona()->activitiesConsumer()->currentActivity())) {
                m_activitiesController->setCurrentActivity(nextActivity);
                return true;
            }
        }
    }

    //! When going from memory usage to different memory usage we first
    //! send the layouts that will be changed. This signal creates the
    //! nice animation that hides these docks/panels
    if (previousMemoryUsage != -1) {
        for (const auto layout : m_centralLayouts) {
            emit currentLayoutIsSwitching(layout->name());
        }

        for (const auto layout : m_sharedLayouts) {
            emit currentLayoutIsSwitching(layout->name());
        }
    }

    QString lPath = layoutPath(layoutName);

    if (lPath.isEmpty() && layoutName == i18n("Alternative")) {
        lPath = m_manager->newLayout(i18n("Alternative"), i18n("Default"));
    }

    if (!lPath.isEmpty()) {
        if (m_manager->memoryUsage() == Types::SingleLayout) {
            //  emit currentLayoutIsSwitching(currentLayoutName());
        } else if (m_manager->memoryUsage() == Types::MultipleLayouts && layoutName != Layout::AbstractLayout::MultipleLayoutsName) {
            CentralLayout toLayout(this, lPath);

            QStringList toActivities = toLayout.activities();

            CentralLayout *centralForOrphans{nullptr};

            for (const auto fromLayout : m_centralLayouts) {
                if (fromLayout->activities().isEmpty()) {
                    centralForOrphans = fromLayout;
                    break;
                }
            }

            if (toActivities.isEmpty() && centralForOrphans && (toLayout.name() != centralForOrphans->name())) {
                emit currentLayoutIsSwitching(centralForOrphans->name());
            }
        }

        //! this code must be called asynchronously because it is called
        //! also from qml (Tasks plasmoid). This change fixes a very important
        //! crash when switching sessions through the Tasks plasmoid Context menu
        //! Latte was unstable and was crashing very often during changing
        //! sessions.
        QTimer::singleShot(350, [this, layoutName, lPath, previousMemoryUsage]() {
            qDebug() << layoutName << " - " << lPath;
            QString fixedLPath = lPath;
            QString fixedLayoutName = layoutName;

            bool initializingMultipleLayouts{false};

            if (m_manager->memoryUsage() == Types::MultipleLayouts && !m_multipleModeInitialized) {
                initializingMultipleLayouts = true;
            }

            if (m_manager->memoryUsage() == Types::SingleLayout || initializingMultipleLayouts || previousMemoryUsage == Types::MultipleLayouts) {
                unloadLayouts();

                if (initializingMultipleLayouts) {
                    fixedLayoutName = QString(Layout::AbstractLayout::MultipleLayoutsName);
                    fixedLPath = layoutPath(fixedLayoutName);
                }

                if (fixedLayoutName != Layout::AbstractLayout::MultipleLayoutsName) {
                    CentralLayout *newLayout = new CentralLayout(this, fixedLPath, fixedLayoutName);
                    addLayout(newLayout);
                }

                m_manager->loadLatteLayout(fixedLPath);

                if (initializingMultipleLayouts) {
                    m_multipleModeInitialized = true;
                }

                emit centralLayoutsChanged();
            }

            if (m_manager->memoryUsage() == Types::MultipleLayouts) {
                if (!initializingMultipleLayouts && !centralLayout(layoutName)) {
                    //! When we are in Multiple Layouts Environment and the user activates
                    //! a Layout that is assigned to specific activities but this
                    //! layout isnt loaded (this means neither of its activities are running)
                    //! is such case we just activate these Activities
                    CentralLayout layout(this, Importer::layoutFilePath(layoutName));

                    int i = 0;
                    bool lastUsedActivityFound{false};
                    QString lastUsedActivity = layout.lastUsedActivity();

                    bool orphanedLayout = !layoutIsAssigned(layoutName);

                    QStringList assignedActivities = orphanedLayout ? orphanedActivities() : layout.activities();

                    if (!orphanedLayout) {
                        for (const auto &assignedActivity : assignedActivities) {
                            //! Starting the activities must be done asynchronous because otherwise
                            //! the activity manager cant close multiple activities
                            QTimer::singleShot(i * 1000, [this, assignedActivity, lastUsedActivity]() {
                                m_activitiesController->startActivity(assignedActivity);

                                if (lastUsedActivity == assignedActivity) {
                                    m_activitiesController->setCurrentActivity(lastUsedActivity);
                                }
                            });

                            if (lastUsedActivity == assignedActivity) {
                                lastUsedActivityFound = true;
                            }

                            i = i + 1;
                        }
                    } else {
                        //! orphaned layout
                        for (const auto &assignedActivity : assignedActivities) {
                            if (lastUsedActivity == assignedActivity) {
                                lastUsedActivityFound = true;
                            }
                        }

                        if ((!lastUsedActivityFound && assignedActivities.count() == 0)
                                || !assignedActivities.contains(m_manager->corona()->activitiesConsumer()->currentActivity())) {

                            //! Starting the activities must be done asynchronous because otherwise
                            //! the activity manager cant close multiple activities
                            QTimer::singleShot(1000, [this, lastUsedActivity, lastUsedActivityFound]() {
                                m_activitiesController->startActivity(lastUsedActivity);
                                m_activitiesController->setCurrentActivity(lastUsedActivity);
                            });
                        }
                    }

                    if (orphanedLayout) {
                        syncMultipleLayoutsToActivities(layoutName);
                    } else if (!orphanedLayout && !lastUsedActivityFound) {
                        m_activitiesController->setCurrentActivity(layout.activities()[0]);
                    }
                } else {
                    syncMultipleLayoutsToActivities(layoutName);
                }
            }

            m_manager->corona()->universalSettings()->setCurrentLayoutName(layoutName);

            if (!layoutIsAssigned(layoutName)) {
                m_manager->corona()->universalSettings()->setLastNonAssignedLayoutName(layoutName);
            }
        });
    } else {
        qDebug() << "Layout : " << layoutName << " was not found...";
    }

    return true;
}

void Synchronizer::syncMultipleLayoutsToActivities(QString layoutForOrphans)
{
    qDebug() << "   ----  --------- ------    syncMultipleLayoutsToActivities       -------   ";
    qDebug() << "   ----  --------- ------    -------------------------------       -------   ";

    QStringList layoutsToUnload;
    QStringList layoutsToLoad;

    bool allRunningActivitiesWillBeReserved{true};

    if (layoutForOrphans.isEmpty() || m_assignedLayouts.values().contains(layoutForOrphans)) {
        layoutForOrphans = m_manager->corona()->universalSettings()->lastNonAssignedLayoutName();
    }

    for (const auto &activity : runningActivities()) {
        if (!m_assignedLayouts[activity].isEmpty()) {
            if (!layoutsToLoad.contains(m_assignedLayouts[activity])) {
                layoutsToLoad.append(m_assignedLayouts[activity]);
            }
        } else {
            allRunningActivitiesWillBeReserved = false;
        }
    }

    for (const auto layout : m_centralLayouts) {
        QString tempLayoutName;

        if (!layoutsToLoad.contains(layout->name()) && layout->name() != layoutForOrphans) {
            tempLayoutName = layout->name();
        } else if (layout->activities().isEmpty() && allRunningActivitiesWillBeReserved) {
            //! in such case the layout for the orphaned must be unloaded
            tempLayoutName = layout->name();
        }

        if (!tempLayoutName.isEmpty() && !layoutsToUnload.contains(tempLayoutName)) {
            layoutsToUnload << tempLayoutName;
        }
    }

    //! Unload no needed Layouts
    for (const auto &layoutName : layoutsToUnload) {
        CentralLayout *layout = centralLayout(layoutName);
        int posLayout = centralLayoutPos(layoutName);

        if (posLayout >= 0) {
            qDebug() << "REMOVING LAYOUT ::::: " << layoutName;
            m_centralLayouts.removeAt(posLayout);

            layout->syncToLayoutFile(true);
            layout->unloadContainments();
            layout->unloadLatteViews();
            m_manager->clearUnloadedContainmentsFromLinkedFile(layout->unloadedContainmentsIds());
            delete layout;
        }
    }

    //! Add Layout for orphan activities
    if (!allRunningActivitiesWillBeReserved) {
        if (!centralLayout(layoutForOrphans)) {
            CentralLayout *newLayout = new CentralLayout(this, layoutPath(layoutForOrphans), layoutForOrphans);

            if (newLayout) {
                qDebug() << "ACTIVATING ORPHANED LAYOUT ::::: " << layoutForOrphans;
                addLayout(newLayout);
                newLayout->importToCorona();
            }
        }
    }

    //! Add needed Layouts based on Activities
    for (const auto &layoutName : layoutsToLoad) {
        if (!centralLayout(layoutName)) {
            CentralLayout *newLayout = new CentralLayout(this, QString(layoutPath(layoutName)), layoutName);

            if (newLayout) {
                qDebug() << "ACTIVATING LAYOUT ::::: " << layoutName;
                addLayout(newLayout);
                newLayout->importToCorona();

                if (m_manager->corona()->universalSettings()->showInfoWindow()) {
                    m_manager->showInfoWindow(i18n("Activating layout: <b>%0</b> ...").arg(newLayout->name()), 5000, newLayout->appliedActivities());
                }
            }
        }
    }

    updateCurrentLayoutNameInMultiEnvironment();
    emit centralLayoutsChanged();
}

void Synchronizer::syncActiveShares(SharesMap &sharesMap, QStringList &deprecatedShares)
{
    if (m_manager->memoryUsage() != Types::MultipleLayouts) {
        return;
    }

    qDebug() << " CURRENT SHARES MAP :: " << sharesMap;
    qDebug() << " DEPRECATED SHARES :: " << deprecatedShares;

    QHash<CentralLayout *, SharedLayout *> unassign;

    //! CENTRAL (inactive) layouts that must update their SharedLayoutName because they
    //! were unassigned from a Shared Layout
    for (const auto &share : deprecatedShares) {
        CentralLayout *central = centralLayout(share);
        if (!central) {
            //! Central Layout is not loaded
            CentralLayout centralInStorage(this, Importer::layoutFilePath(share));
            centralInStorage.setSharedLayoutName(QString());
        }
    }

    //! CENTRAL (active) layouts that will become SHARED must be unloaded first
    for (SharesMap::iterator i=sharesMap.begin(); i!=sharesMap.end(); ++i) {
        CentralLayout *central = centralLayout(i.key());
        if (central) {
            unloadCentralLayout(central);
        }
    }

    //! CENTRAL (active) layouts that update their (active) SHARED layouts
    //! AND load SHARED layouts that are NOT ACTIVE
    for (SharesMap::iterator i=sharesMap.begin(); i!=sharesMap.end(); ++i) {
        SharedLayout *shared = sharedLayout(i.key());
        qDebug() << " SHARED :: " << i.key();
        for (const auto &centralName : i.value()) {
            CentralLayout *central = centralLayout(centralName);
            qDebug() << " CENTRAL NAME :: " << centralName;
            if (central) {
                //! Assign this Central Layout at a different Shared Layout
                SharedLayout *oldShared = central->sharedLayout();

                if (!shared) {
                    //Shared not loaded and it must be loaded before proceed
                    registerAtSharedLayout(central, i.key());
                    shared = sharedLayout(i.key());
                }

                if (shared != oldShared) {
                    shared->addCentralLayout(central);
                    central->setSharedLayout(shared);
                    if (oldShared) {
                        //! CENTRAL layout that changed from one ACTIVESHARED layout to another
                        unassign[central] = shared;
                    }
                }
            } else {
                //! Central Layout is not loaded
                CentralLayout centralInStorage(this, Importer::layoutFilePath(centralName));
                centralInStorage.setSharedLayoutName(i.key());
            }
        }
    }

    //! CENTRAL Layouts that wont have any SHARED Layout any more
    for (const auto &centralName : centralLayoutsNames()) {
        if (!mapHasRecord(centralName, sharesMap)) {
            CentralLayout *central = centralLayout(centralName);
            if (central && central->sharedLayout()) {
                central->sharedLayout()->removeCentralLayout(central);
                central->setSharedLayoutName(QString());
                central->setSharedLayout(nullptr);
            }
        }
    }

    //! Unassing from Shared Layouts Central ones that are not assigned any more
    //! IMPORTANT: This must be done after all the ASSIGNMENTS in order to avoid
    //! to unload a SharedLayout that it should not
    for (QHash<CentralLayout *, SharedLayout *>::iterator i=unassign.begin(); i!=unassign.end(); ++i) {
        i.value()->removeCentralLayout(i.key());
    }
}

}
} // end of namespace
