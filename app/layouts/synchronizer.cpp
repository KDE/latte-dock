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
#include "../apptypes.h"
#include "../data/layoutdata.h"
#include "../lattecorona.h"
#include "../layout/centrallayout.h"
#include "../layout/genericlayout.h"
#include "../settings/universalsettings.h"
#include "../templates/templatesmanager.h"
#include "../view/view.h"

// Qt
#include <QDir>
#include <QFile>

// Plasma
#include <Plasma/Containment>

// KDE
#include <KActivities/Consumer>
#include <KActivities/Controller>
#include <KWindowSystem>

namespace Latte {
namespace Layouts {

Synchronizer::Synchronizer(QObject *parent)
    : QObject(parent),
      m_activitiesController(new KActivities::Controller)
{
    m_manager = qobject_cast<Manager *>(parent);

    //! KActivities tracking
    connect(m_manager->corona()->activitiesConsumer(), &KActivities::Consumer::currentActivityChanged,
            this, &Synchronizer::currentActivityChanged);

    connect(m_manager->corona()->activitiesConsumer(), &KActivities::Consumer::runningActivitiesChanged,
            this, [&]() {
        if (m_manager->memoryUsage() == MemoryUsage::MultipleLayouts) {
            syncMultipleLayoutsToActivities();
        }
    });
}

Synchronizer::~Synchronizer()
{
    m_activitiesController->deleteLater();
}

KActivities::Controller *Synchronizer::activitiesController() const
{
    return m_activitiesController;
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


bool Synchronizer::isAssigned(QString layoutName) const
{
    for(auto activityid : m_assignedLayouts.keys()) {
        if (m_assignedLayouts[activityid].contains(layoutName)) {
            return true;
        }
    }

    return false;
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
    if (m_manager->memoryUsage() == MemoryUsage::SingleLayout) {
        return m_manager->corona()->universalSettings()->currentLayoutName();
    } else if (m_manager->memoryUsage() == MemoryUsage::MultipleLayouts) {
        return currentLayoutNameInMultiEnvironment();
    }

    return QString();
}

QString Synchronizer::currentLayoutNameInMultiEnvironment() const
{
    return m_currentLayoutNameInMultiEnvironment;
}

void Synchronizer::setCurrentLayoutNameInMultiEnvironment(const QString &name)
{
    if (m_currentLayoutNameInMultiEnvironment == name) {
        return;
    }

    m_currentLayoutNameInMultiEnvironment = name;
    emit currentLayoutNameChanged();
}

QString Synchronizer::layoutPath(QString layoutName)
{
    QString path = Layouts::Importer::layoutUserFilePath(layoutName);

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

QStringList Synchronizer::freeActivities()
{
    QStringList fActivities;

    for (const auto &activity : runningActivities()) {
        if (!m_assignedLayouts.contains(activity)) {
            fActivities.append(activity);
        }
    }

    return fActivities;
}

QStringList Synchronizer::centralLayoutsNames()
{
    QStringList names;

    if (m_manager->memoryUsage() == MemoryUsage::SingleLayout) {
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
    if (!fixedMenuLayouts.contains(currentLayoutName()) && m_manager->memoryUsage() == MemoryUsage::SingleLayout) {
        fixedMenuLayouts.prepend(currentLayoutName());
    } else if (m_manager->memoryUsage() == MemoryUsage::MultipleLayouts) {
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
    if (m_manager->memoryUsage() == MemoryUsage::SingleLayout) {
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

    return l;
}

Latte::View *Synchronizer::viewForContainment(Plasma::Containment *containment)
{
    for (auto layout : m_centralLayouts) {
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

void Synchronizer::currentActivityChanged(const QString &id)
{
    if (m_manager->memoryUsage() == MemoryUsage::MultipleLayouts) {
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
    if (m_manager->memoryUsage() == MemoryUsage::MultipleLayouts) {
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
    if (m_manager->memoryUsage() == MemoryUsage::MultipleLayouts) {
        for (const auto layout : m_centralLayouts) {
            layout->syncToLayoutFile();
        }
    }
}

void Synchronizer::syncLatteViewsToScreens()
{
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

        central->unloadLatteViews();
        central->unloadContainments();

        if (m_multipleModeInitialized) {
            m_manager->clearUnloadedContainmentsFromLinkedFile(central->unloadedContainmentsIds(), true);
        }

        delete central;
    }
}

void Synchronizer::loadLayouts()
{
    m_layouts.clear();
    m_menuLayouts.clear();
    m_assignedLayouts.clear();

    QDir layoutDir(Layouts::Importer::layoutUserDir());
    QStringList filter;
    filter.append(QString("*.layout.latte"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    for (const auto &layout : files) {
        if (layout.contains(Layout::MULTIPLELAYOUTSHIDDENNAME)) {
            //! IMPORTANT: DON'T ADD MultipleLayouts hidden file in layouts list
            continue;
        }

        QString layoutpath = layoutDir.absolutePath() + "/" + layout;
        onLayoutAdded(layoutpath);
    }

    m_layouts.sort(Qt::CaseInsensitive);
    m_menuLayouts.sort(Qt::CaseInsensitive);

    emit layoutsChanged();
    emit menuLayoutsChanged();

    if (!m_isLoaded) {
        m_isLoaded = true;
        connect(m_manager->corona()->templatesManager(), &Latte::Templates::Manager::newLayoutAdded, this, &Synchronizer::onLayoutAdded);
    }
}

void Synchronizer::onLayoutAdded(const QString &layout)
{
    CentralLayout centralLayout(this, layout);

    for (const auto &activity : centralLayout.activities()) {
        if (m_assignedLayouts.contains(activity)) {
            m_assignedLayouts[activity] << centralLayout.name();
        } else {
            m_assignedLayouts[activity] = QStringList(centralLayout.name());
        }
    }

    m_layouts.append(centralLayout.name());

    if (centralLayout.showInMenu()) {
        m_menuLayouts.append(centralLayout.name());
    }

    if (m_isLoaded) {
        m_layouts.sort(Qt::CaseInsensitive);
        m_menuLayouts.sort(Qt::CaseInsensitive);

        emit layoutsChanged();
        emit menuLayoutsChanged();
    }
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
            setCurrentLayoutNameInMultiEnvironment(layout->name());
            return;
        }
    }

    for (const auto layout : m_centralLayouts) {
        if (layout->activities().isEmpty()) {
            setCurrentLayoutNameInMultiEnvironment(layout->name());
            return;
        }
    }
}

bool Synchronizer::switchToLayout(QString layoutName, int previousMemoryUsage)
{
    if (m_centralLayouts.size() > 0 && currentLayoutName() == layoutName && previousMemoryUsage == -1) {
        return false;
    }

    //! First Check If that Layout is already present and in that case
    //! we can just switch to the proper Activity
    if (m_manager->memoryUsage() == MemoryUsage::MultipleLayouts && previousMemoryUsage == -1) {
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
    }

    QString lPath = layoutPath(layoutName);

    if ((m_manager->memoryUsage() == MemoryUsage::SingleLayout && !lPath.isEmpty()) || m_manager->memoryUsage() == MemoryUsage::MultipleLayouts) {
        if (m_manager->memoryUsage() == MemoryUsage::SingleLayout) {
            emit currentLayoutIsSwitching(currentLayoutName());
        } else if (m_manager->memoryUsage() == MemoryUsage::MultipleLayouts && layoutName != Layout::MULTIPLELAYOUTSHIDDENNAME) {
            //! do nothing
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

            bool initializingMultipleLayouts{(m_manager->memoryUsage() == MemoryUsage::MultipleLayouts) && !m_multipleModeInitialized};

            if (m_manager->memoryUsage() == MemoryUsage::SingleLayout || initializingMultipleLayouts || previousMemoryUsage == MemoryUsage::MultipleLayouts) {
                unloadLayouts();

                //! load the main layout/corona file
                if (initializingMultipleLayouts) {
                    fixedLayoutName = QString(Layout::MULTIPLELAYOUTSHIDDENNAME);
                    fixedLPath = layoutPath(fixedLayoutName);
                } else if (fixedLayoutName != Layout::MULTIPLELAYOUTSHIDDENNAME) {
                    CentralLayout *newLayout = new CentralLayout(this, fixedLPath, fixedLayoutName);
                    addLayout(newLayout);
                }

                m_manager->loadLatteLayout(fixedLPath);

                if (initializingMultipleLayouts) {
                    m_multipleModeInitialized = true;
                }

                emit centralLayoutsChanged();
            }

            if (m_manager->memoryUsage() == MemoryUsage::MultipleLayouts) {
                syncMultipleLayoutsToActivities();
            }

            m_manager->corona()->universalSettings()->setCurrentLayoutName(layoutName);

            if (!isAssigned(layoutName)) {
                m_manager->corona()->universalSettings()->setLastNonAssignedLayoutName(layoutName);
            }
        });
    } else {
        qDebug() << "Layout : " << layoutName << " was not found...";
    }

    return true;
}

void Synchronizer::syncMultipleLayoutsToActivities()
{
    qDebug() << "   ----  --------- ------    syncMultipleLayoutsToActivities       -------   ";
    qDebug() << "   ----  --------- ------    -------------------------------       -------   ";

    QStringList layoutsToUnload;
    QStringList layoutsToLoad;

    //! discover OnAllActivities layouts
    if (m_assignedLayouts.contains(Data::Layout::ALLACTIVITIESID)) {
        layoutsToLoad << m_assignedLayouts[Data::Layout::ALLACTIVITIESID];
    }

    //! discover ForFreeActivities layouts
    if (m_assignedLayouts.contains(Data::Layout::FREEACTIVITIESID)) {
        layoutsToLoad << m_assignedLayouts[Data::Layout::FREEACTIVITIESID];
    }

    //! discover layouts assigned to explicit activities based on running activities
    for (const auto &activity : runningActivities()) {
        if (KWindowSystem::isPlatformWayland() && (m_activitiesController->currentActivity() != activity)){
            //! Wayland Protection: Plasma wayland does not support yet Activities for windows
            //! but we can load the layouts that belong OnAllActivities + (ForFreeActivities OR SpecificActivity)
            continue;
        }

        if (m_assignedLayouts.contains(activity)) {
            layoutsToLoad << m_assignedLayouts[activity];
        }
    }

    //! discover layouts that must be unloaded because of running activities changes
    for (const auto layout : m_centralLayouts) {
        if (!layoutsToLoad.contains(layout->name())) {
            layoutsToUnload << layout->name();
        }
    }

    QString defaultForcedLayout;

    //! Safety
    if (layoutsToLoad.isEmpty()) {
        //! If no layout is found then force loading Default Layout
        QString layoutPath = m_manager->corona()->templatesManager()->newLayout("", i18n(Templates::DEFAULTLAYOUTTEMPLATENAME));
        layoutsToLoad << Layout::AbstractLayout::layoutName(layoutPath);
        defaultForcedLayout = layoutsToLoad[0];
    }

    //! Add needed Layouts based on Activities settings
    for (const auto &layoutName : layoutsToLoad) {
        if (!centralLayout(layoutName)) {
            CentralLayout *newLayout = new CentralLayout(this, QString(layoutPath(layoutName)), layoutName);

            if (newLayout) {
                qDebug() << "ACTIVATING LAYOUT ::::: " << layoutName;
                addLayout(newLayout);
                newLayout->importToCorona();

                if (!defaultForcedLayout.isEmpty() && newLayout->name() == defaultForcedLayout) {
                    newLayout->setActivities(QStringList(Data::Layout::ALLACTIVITIESID));
                }

                if (m_manager->corona()->universalSettings()->showInfoWindow()) {
                    m_manager->showInfoWindow(i18n("Activating layout: <b>%0</b> ...").arg(newLayout->name()), 5000, newLayout->appliedActivities());
                }
            }
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

    updateCurrentLayoutNameInMultiEnvironment();
    emit centralLayoutsChanged();
}

}
} // end of namespace
