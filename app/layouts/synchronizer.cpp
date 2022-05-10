/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "synchronizer.h"

//! local
#include <config-latte.h>
#include "importer.h"
#include "manager.h"
#include "../apptypes.h"
#include "../screenpool.h"
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
#include <QStringList>

// Plasma
#include <Plasma/Containment>

// KDE
#include <KActivities/Consumer>
#include <KActivities/Controller>
#include <KWindowSystem>

#define LAYOUTSINITINTERVAL 350

namespace Latte {
namespace Layouts {

Synchronizer::Synchronizer(QObject *parent)
    : QObject(parent),
      m_activitiesController(new KActivities::Controller)
{
    m_manager = qobject_cast<Manager *>(parent);

    connect(this, &Synchronizer::layoutsChanged, this, &Synchronizer::reloadAssignedLayouts);

    //! KWin update Disabled Borders
    connect(this, &Synchronizer::centralLayoutsChanged, this, &Synchronizer::updateBorderlessMaximizedAfterTimer);
    connect(m_manager->corona()->universalSettings(), &UniversalSettings::canDisableBordersChanged, this, &Synchronizer::updateKWinDisabledBorders);

    m_updateBorderlessMaximized.setInterval(500);
    m_updateBorderlessMaximized.setSingleShot(true);
    connect(&m_updateBorderlessMaximized, &QTimer::timeout, this, &Synchronizer::updateKWinDisabledBorders);

    //! KActivities tracking
    connect(m_manager->corona()->activitiesConsumer(), &KActivities::Consumer::activityRemoved,
            this, &Synchronizer::onActivityRemoved);

    connect(m_manager->corona()->activitiesConsumer(), &KActivities::Consumer::currentActivityChanged,
            this, [&]() {
        if (m_manager->memoryUsage() == MemoryUsage::MultipleLayouts) {
            //! this signal is also triggered when runningactivities are changed and actually is received first
            //! this is why we need a timer here in order to delay that execution and not activate/deactivate
            //! maximizedborders faulty because syncMultipleLayoutsToActivities(); has not been executed yet
            updateBorderlessMaximizedAfterTimer();
        }
    });

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
    return m_layouts.containsName(layoutName);
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

QStringList Synchronizer::freeActivities()
{
    QStringList frees = activities();

    for(auto assigned : m_assignedLayouts.keys()) {
        frees.removeAll(assigned);
    }

    return frees;
}

QStringList Synchronizer::runningActivities()
{   
    return m_manager->corona()->activitiesConsumer()->runningActivities();
}

QStringList Synchronizer::freeRunningActivities()
{
    QStringList fActivities;

    for (const auto &activity : runningActivities()) {
        if (!m_assignedLayouts.contains(activity)) {
            fActivities.append(activity);
        }
    }

    return fActivities;
}

QStringList Synchronizer::validActivities(const QStringList &layoutActivities)
{
    QStringList valids;
    QStringList allactivities = activities();

    for(auto activity : layoutActivities) {
        if (allactivities.contains(activity)) {
            valids << activity;
        }
    }

    return valids;
}

QStringList Synchronizer::centralLayoutsNames()
{
    QStringList names;

    if (m_manager->memoryUsage() == MemoryUsage::SingleLayout) {
        if (m_centralLayouts.count() > 0) {
            names << m_centralLayouts.at(0)->name();
        }
    } else {
        for (int i = 0; i < m_centralLayouts.size(); ++i) {
            CentralLayout *layout = m_centralLayouts.at(i);
            names << layout->name();
        }
    }

    return names;
}

QStringList Synchronizer::currentLayoutsNames() const
{
    QList<CentralLayout *> currents = currentLayouts();
    QStringList currentNames;

    for (int i = 0; i < currents.size(); ++i) {
        CentralLayout *layout = currents.at(i);
        currentNames << layout->name();
    }

    return currentNames;
}

QStringList Synchronizer::layouts() const
{
    return m_layouts.names();
}

QStringList Synchronizer::menuLayouts() const
{
    QStringList menulayouts;

    for (int i=0; i<m_layouts.rowCount(); ++i) {
        if (!m_layouts[i].isShownInMenu) {
            continue;
        }

        menulayouts << m_layouts[i].name;
    }

    for (const auto layout : m_centralLayouts) {
        if (!menulayouts.contains(layout->name())) {
            menulayouts.prepend(layout->name());
        }
    }

    menulayouts.sort(Qt::CaseInsensitive);

    return menulayouts;
}

void Synchronizer::setIsSingleLayoutInDeprecatedRenaming(const bool &enabled)
{
    m_isSingleLayoutInDeprecatedRenaming = enabled;
}

Data::Layout Synchronizer::data(const QString &storedLayoutName) const
{
    Data::Layout l;

    if (m_layouts.containsName(storedLayoutName)) {
        QString lid = m_layouts.idForName(storedLayoutName);
        return m_layouts[lid];
    }

    return l;
}

Data::LayoutsTable Synchronizer::layoutsTable() const
{
    return m_layouts;
}

void Synchronizer::setLayoutsTable(const Data::LayoutsTable &table)
{
    if (m_layouts == table) {
        return;
    }

    m_layouts = table;
    emit layoutsChanged();
}

void Synchronizer::updateLayoutsTable()
{
    for (int i = 0; i < m_centralLayouts.size(); ++i) {
        CentralLayout *layout = m_centralLayouts.at(i);

        if (m_layouts.containsId(layout->file())) {
            m_layouts[layout->file()] = layout->data();
        }
    }

    for (int i = 0; i < m_layouts.rowCount(); ++i) {
        if ((m_layouts[i].errors>0 || m_layouts[i].warnings>0) && !m_layouts[i].isActive) {
            CentralLayout central(this, m_layouts[i].id);
            m_layouts[i].errors = central.errors().count();
            m_layouts[i].warnings = central.warnings().count();
        }
    }
}

CentralLayout *Synchronizer::centralLayout(QString layoutname) const
{
    for (int i = 0; i < m_centralLayouts.size(); ++i) {
        CentralLayout *layout = m_centralLayouts.at(i);

        if (layout->name() == layoutname) {
            return layout;
        }
    }

    return nullptr;
}

QList<CentralLayout *> Synchronizer::currentLayouts() const
{
    QList<CentralLayout *> layouts;
    layouts.clear();

    if (m_centralLayouts.isEmpty()) {
        return layouts;
    }

    if (m_manager->memoryUsage() == MemoryUsage::SingleLayout) {
        layouts << m_centralLayouts[0];
    } else {
        for (auto layout : m_centralLayouts) {
            if (layout->isOnAllActivities() || layout->appliedActivities().contains(m_manager->corona()->activitiesConsumer()->currentActivity())) {
                layouts << layout;
            }
        }
    }

    return layouts;
}

QList<CentralLayout *> Synchronizer::centralLayoutsForActivity(const QString activityid) const
{
    QList<CentralLayout *> layouts;

    if (m_manager->memoryUsage() == MemoryUsage::SingleLayout && m_centralLayouts.count() >= 1) {
        layouts << m_centralLayouts.at(0);
    } else {
        for (auto layout : m_centralLayouts) {
            if (layout->isOnAllActivities() || layout->appliedActivities().contains(activityid)) {
                layouts << layout;
            }
        }
    }

    return layouts;
}

QList<Latte::View *> Synchronizer::currentViews() const
{
    QList<Latte::View *> views;

    for(auto layout : currentLayouts()) {
        views << layout->latteViews();
    }

    return views;
}

QList<Latte::View *> Synchronizer::currentOriginalViews() const
{
    QList<Latte::View *> views;

    for(auto layout : currentLayouts()) {
        views << layout->onlyOriginalViews();
    }

    return views;
}

QList<Latte::View *> Synchronizer::currentViewsWithPlasmaShortcuts() const
{
    QList<Latte::View *> views;

    for(auto layout : currentLayouts()) {
        views << layout->viewsWithPlasmaShortcuts();
    }

    return views;
}

QList<Latte::View *> Synchronizer::sortedCurrentViews() const
{
    return Layout::GenericLayout::sortedLatteViews(currentViews(), m_manager->corona()->screenPool()->primaryScreen());
}

QList<Latte::View *> Synchronizer::sortedCurrentOriginalViews() const
{
    return Layout::GenericLayout::sortedLatteViews(currentOriginalViews(), m_manager->corona()->screenPool()->primaryScreen());
}

QList<Latte::View *> Synchronizer::viewsBasedOnActivityId(const QString &id) const
{
    QList<Latte::View *> views;

    for(auto layout : centralLayoutsForActivity(id)) {
        if (m_centralLayouts.contains(layout)) {
            views << layout->latteViews();
        }
    }

    return views;
}

Layout::GenericLayout *Synchronizer::layout(QString layoutname) const
{
    Layout::GenericLayout *l = centralLayout(layoutname);

    return l;
}

int Synchronizer::screenForContainment(Plasma::Containment *containment)
{
    for (auto layout : m_centralLayouts) {
        if (layout->contains(containment)) {
            return layout->screenForContainment(containment);
        }
    }

    return -1;
}

Latte::View *Synchronizer::viewForContainment(uint id)
{
    for (auto layout : m_centralLayouts) {
        Latte::View *view = layout->viewForContainment(id);

        if (view) {
            return view;
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

    return nullptr;
}

void Synchronizer::addLayout(CentralLayout *layout)
{
    if (!m_centralLayouts.contains(layout)) {
        m_centralLayouts.append(layout);
    }
}

void Synchronizer::onActivityRemoved(const QString &activityid)
{
    if (!m_assignedLayouts.contains(activityid)) {
        return;
    }

    //! remove any other explicit set layouts for the current activity
    QStringList explicits = m_assignedLayouts[activityid];

    for(auto explicitlayoutname : explicits) {
        QString explicitlayoutid = m_layouts.idForName(explicitlayoutname);

        m_layouts[explicitlayoutid].activities.removeAll(activityid);
        m_manager->setOnActivities(explicitlayoutname, m_layouts[explicitlayoutid].activities);
        emit layoutActivitiesChanged(m_layouts[explicitlayoutid]);
    }

    QStringList freelayoutnames;

    if (m_assignedLayouts.contains(Data::Layout::FREEACTIVITIESID)) {
        freelayoutnames = m_assignedLayouts[Data::Layout::FREEACTIVITIESID];
    }

    reloadAssignedLayouts();

    for(auto freelayoutname : freelayoutnames) {
        //! inform free activities layouts that their activities probably changed
        CentralLayout *central = centralLayout(freelayoutname);

        if (central) {
            emit central->activitiesChanged();
        }
    }
}

void Synchronizer::updateBorderlessMaximizedAfterTimer()
{
    //! this signal is also triggered when runningactivities are changed and actually is received first
    //! this is why we need a timer here in order to delay that execution and not activate/deactivate
    //! maximizedborders faulty because syncMultipleLayoutsToActivities(); has not been executed yet
    m_updateBorderlessMaximized.start();
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

        if (layout->isOnAllActivities()) {
            return;
        }

        QStringList appliedactivities = layout->appliedActivities();

        if (layout && !appliedactivities.isEmpty()) {
            int i = 0;

            for (const auto &activityid : appliedactivities) {
                //! Stopping the activities must be done asynchronous because otherwise
                //! the activity manager cant close multiple activities
                QTimer::singleShot(i * 1000, [this, activityid]() {
                    m_activitiesController->stopActivity(activityid);
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
        CentralLayout *central = m_centralLayouts.takeAt(pos);

        if (m_multipleModeInitialized && !m_manager->corona()->inQuit()) {
            central->syncToLayoutFile(true);
        }

        central->unloadLatteViews();
        central->unloadContainments();

        if (m_multipleModeInitialized && !m_manager->corona()->inQuit()) {
            m_manager->clearUnloadedContainmentsFromLinkedFile(central->unloadedContainmentsIds(), true);
        }

        delete central;
    }
}

void Synchronizer::initLayouts()
{
    m_layouts.clear();

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

    emit layoutsChanged();

    if (!m_isLoaded) {
        m_isLoaded = true;
        connect(m_manager->corona()->templatesManager(), &Latte::Templates::Manager::newLayoutAdded, this, &Synchronizer::onLayoutAdded);
        connect(m_manager->importer(), &Latte::Layouts::Importer::newLayoutAdded, this, &Synchronizer::onLayoutAdded);
    }
}

void Synchronizer::onLayoutAdded(const QString &layout)
{
    CentralLayout centrallayout(this, layout);
    m_layouts.insertBasedOnName(centrallayout.data());

    if (m_isLoaded) {
        emit layoutsChanged();
    }
}

void Synchronizer::reloadAssignedLayouts()
{
    m_assignedLayouts.clear();

    for (int i=0; i< m_layouts.rowCount(); ++i) {
        for (const auto &activity : m_layouts[i].activities) {
            if (m_assignedLayouts.contains(activity)) {
                m_assignedLayouts[activity] << m_layouts[i].name;
            } else {
                m_assignedLayouts[activity] = QStringList(m_layouts[i].name);
            }
        }
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

void Synchronizer::unloadPreloadedLayouts()
{
    QStringList currentnames;
    QStringList preloadednames = Layouts::Storage::self()->storedLayoutsInMultipleFile();

    for(auto l : m_centralLayouts) {
        if (l) {
            currentnames << l->name();
        }
    }

    for(auto lname : preloadednames) {
        if (!currentnames.contains(lname)) {
            Layouts::Storage::self()->moveToLayoutFile(lname);
        }
    }
}

bool Synchronizer::memoryInitialized() const
{
    return ((m_manager->memoryUsage() == MemoryUsage::SingleLayout && m_centralLayouts.size()>0)
            || (m_manager->memoryUsage() == MemoryUsage::MultipleLayouts && m_multipleModeInitialized));
}

bool Synchronizer::initSingleMode(QString layoutName)
{
    QString layoutpath = layoutName.isEmpty() ? layoutPath(m_manager->corona()->universalSettings()->singleModeLayoutName()) : layoutPath(layoutName);

    if (layoutpath.isEmpty()) {
        qDebug() << "Layout : " << layoutName << " was not found...";
        return false;
    }

    if (m_centralLayouts.size() > 0) {
        emit currentLayoutIsSwitching(m_centralLayouts[0]->name());
    }

    //! this code must be called asynchronously because it can create crashes otherwise.
    //! Tasks plasmoid case that triggers layouts switching through its context menu
    QTimer::singleShot(LAYOUTSINITINTERVAL, [this, layoutName, layoutpath]() {
        qDebug() << " ... initializing layout in single mode : " << layoutName << " - " << layoutpath;
        unloadPreloadedLayouts();
        unloadLayouts();

        //! load the main single layout/corona file
        CentralLayout *newLayout = new CentralLayout(this, layoutpath, layoutName);

        //! Order of initialization steps is very important and guarantees correct startup initialization
        //! Step1: corona is set for the layout
        //! Step2: containments from file are loaded into main corona
        //! Step3: layout connects to corona signals and slots
        //! Step4: layout is added in manager and is accesible for others to find
        //! Step5: layout is attaching its initial containmens and is now considered ACTIVE
        newLayout->setCorona(m_manager->corona()); //step1
        m_manager->loadLatteLayout(layoutpath);    //step2
        newLayout->initCorona();                   //step3
        addLayout(newLayout);                      //step4
        newLayout->initContainments();             //step5

        emit centralLayoutsChanged();

        if (m_isSingleLayoutInDeprecatedRenaming) {
            QString deprecatedlayoutpath = layoutPath(m_manager->corona()->universalSettings()->singleModeLayoutName());

            if (!deprecatedlayoutpath.isEmpty()) {
                qDebug() << "Removing Deprecated single layout after renaming:: " << m_manager->corona()->universalSettings()->singleModeLayoutName();
                QFile(deprecatedlayoutpath).remove();
            }

            m_isSingleLayoutInDeprecatedRenaming = false;
        }

        m_manager->corona()->universalSettings()->setSingleModeLayoutName(layoutName);
        m_manager->importer()->setMultipleLayoutsStatus(Latte::MultipleLayouts::Uninitialized);
        emit initializationFinished();
    });

    return true;
}

bool Synchronizer::initMultipleMode(QString layoutName)
{
    if (m_multipleModeInitialized) {
        return false;
    }

    for (const auto layout : m_centralLayouts) {
        emit currentLayoutIsSwitching(layout->name());
    }

    //! this code must be called asynchronously because it can create crashes otherwise.
    //! Tasks plasmoid case that triggers layouts switching through its context menu
    QTimer::singleShot(LAYOUTSINITINTERVAL, [this, layoutName]() {
        qDebug() << " ... initializing layout in multiple mode : " << layoutName ;
        unloadLayouts();

        QStringList layoutsinmultiplestorage = Layouts::Storage::self()->storedLayoutsInMultipleFile();
        qDebug() << "Preloaded Multiple Layouts in Storage :: " << layoutsinmultiplestorage;

        m_manager->loadLatteLayout(layoutPath(QString(Layout::MULTIPLELAYOUTSHIDDENNAME)));

        m_multipleModeInitialized = true;

        emit centralLayoutsChanged();

        if (!layoutName.isEmpty()) {
            switchToLayoutInMultipleModeBasedOnActivities(layoutName);
        }

        syncMultipleLayoutsToActivities(layoutsinmultiplestorage);
        m_manager->importer()->setMultipleLayoutsStatus(Latte::MultipleLayouts::Running);
        emit initializationFinished();
    });

    return true;
}

bool Synchronizer::switchToLayoutInSingleMode(QString layoutName)
{
    if (!memoryInitialized() || m_manager->memoryUsage() != MemoryUsage::SingleLayout) {
        return false;
    }

    if (m_centralLayouts.size()>0 && m_centralLayouts[0]->name() == layoutName) {
        return true;
    }

    return initSingleMode(layoutName);
}

bool Synchronizer::switchToLayoutInMultipleModeBasedOnActivities(const QString &layoutName)
{
    Data::Layout layoutdata;
    CentralLayout *central = centralLayout(layoutName);

    if (central) {
        layoutdata = central->data();
    } else if (m_layouts.containsName(layoutName)) {
        QString layoutid = m_layouts.idForName(layoutName);
        CentralLayout storagedlayout(this, layoutid);
        layoutdata = storagedlayout.data();

        m_layouts[layoutid] = layoutdata;
    }

    if (layoutdata.isEmpty()) {
        return false;
    }

    QString switchToActivity;

    //! try to not remove activityids that belong to different machines that are not currently present
    QStringList validlayoutactivities = validActivities(layoutdata.activities);

    if (layoutdata.isOnAllActivities()) {
        //! no reason to switch in any activity;
    } else if (layoutdata.isForFreeActivities()) {
        //! free-activities case
        QStringList freerunningactivities = freeRunningActivities();

        if (freerunningactivities.count() > 0) {
            if (freerunningactivities.contains(layoutdata.lastUsedActivity)) {
                switchToActivity = layoutdata.lastUsedActivity;
            } else {
                switchToActivity = freerunningactivities[0];
            }
        } else {
            QStringList freepausedactivities = freeActivities();

            if (freepausedactivities.count() > 0) {
                switchToActivity = freepausedactivities[0];
            }
        }
    } else if (!validlayoutactivities.isEmpty())  {
        //! set on-explicit activities
        QStringList allactivities = activities();

        if (validlayoutactivities.contains(layoutdata.lastUsedActivity)) {
            switchToActivity = layoutdata.lastUsedActivity;
        } else {
            switchToActivity = validlayoutactivities[0];
        }
    } else if (validlayoutactivities.isEmpty() && m_layouts.containsName(layoutName)) {
        //! no-activities are set
        //! has not been set in any activities but nonetheless it is requested probably by the user
        //! requested layout is assigned explicitly in current activity and any remaining explicit layouts
        //! are removing current activity from their activities list
        QString layoutid = m_layouts.idForName(layoutName);
        QString currentactivityid = m_activitiesController->currentActivity();

        QStringList layoutIdsChanged;

        m_layouts[layoutid].activities.append(currentactivityid);
        m_manager->setOnActivities(layoutName, m_layouts[layoutid].activities);
        emit layoutActivitiesChanged(m_layouts[layoutid]);

        layoutIdsChanged << layoutid;

        if (m_assignedLayouts.contains(currentactivityid)) {
            //! remove any other explicit set layouts for the current activity
            QStringList explicits = m_assignedLayouts[currentactivityid];

            for(auto explicitlayoutname : explicits) {
                QString explicitlayoutid = m_layouts.idForName(explicitlayoutname);

                m_layouts[explicitlayoutid].activities.removeAll(currentactivityid);
                m_manager->setOnActivities(explicitlayoutname, m_layouts[explicitlayoutid].activities);
                emit layoutActivitiesChanged(m_layouts[explicitlayoutid]);
            }
        }

        QStringList freelayoutnames;
        if (m_assignedLayouts.contains(Data::Layout::FREEACTIVITIESID)) {
            freelayoutnames = m_assignedLayouts[Data::Layout::FREEACTIVITIESID];
        }

        reloadAssignedLayouts();

        for(auto freelayoutname : freelayoutnames) {
            //! inform free activities layouts that their activities probably changed
            CentralLayout *central = centralLayout(freelayoutname);

            if (central) {
                emit central->activitiesChanged();
            }
        }
    }

    if (!switchToActivity.isEmpty()) {
        if (!m_manager->corona()->activitiesConsumer()->runningActivities().contains(switchToActivity)) {
            m_activitiesController->startActivity(switchToActivity);
        }

        m_activitiesController->setCurrentActivity(switchToActivity);
    }

    return true;
}

bool Synchronizer::switchToLayoutInMultipleMode(QString layoutName)
{
    if (!memoryInitialized() || m_manager->memoryUsage() != MemoryUsage::MultipleLayouts) {
        return false;
    }

    CentralLayout *layout = centralLayout(layoutName);

    if (layout) {
        QStringList appliedActivities = layout->appliedActivities();
        QString nextActivity = !layout->lastUsedActivity().isEmpty() ? layout->lastUsedActivity() : appliedActivities[0];

        if (!appliedActivities.contains(m_manager->corona()->activitiesConsumer()->currentActivity())) {
            //! it means we are at a foreign activity and we can switch to correct one
            m_activitiesController->setCurrentActivity(nextActivity);
            return true;
        }
    } else {
        if (!layoutName.isEmpty()) {
            switchToLayoutInMultipleModeBasedOnActivities(layoutName);
        }

        syncMultipleLayoutsToActivities();
    }

    return true;
}


bool Synchronizer::switchToLayout(QString layoutName, MemoryUsage::LayoutsMemory newMemoryUsage)
{
    qDebug() << " >>>>> SWITCHING >> " << layoutName << " __ from memory: " << m_manager->memoryUsage() << " to memory: " << newMemoryUsage;

    if (newMemoryUsage == MemoryUsage::Current) {
        newMemoryUsage = m_manager->memoryUsage();
    }

    if (!memoryInitialized() || newMemoryUsage != m_manager->memoryUsage()) {
        //! Initiate Layouts memory properly
        m_manager->setMemoryUsage(newMemoryUsage);

        return (newMemoryUsage == MemoryUsage::SingleLayout ? initSingleMode(layoutName) : initMultipleMode(layoutName));
    }

    if (m_manager->memoryUsage() == MemoryUsage::SingleLayout) {
        return switchToLayoutInSingleMode(layoutName);
    } else {
        return switchToLayoutInMultipleMode(layoutName);
    }
}

void Synchronizer::syncMultipleLayoutsToActivities(QStringList preloadedLayouts)
{
    qDebug() << "   ----  --------- ------    syncMultipleLayoutsToActivities       -------   ";
    qDebug() << "   ----  --------- ------    -------------------------------       -------   ";

    QStringList layoutNamesToUnload;
    QStringList layoutNamesToLoad;
    QStringList currentNames = centralLayoutsNames();

    //! discover OnAllActivities layouts
    if (m_assignedLayouts.contains(Data::Layout::ALLACTIVITIESID)) {
        layoutNamesToLoad << m_assignedLayouts[Data::Layout::ALLACTIVITIESID];
    }

    //! discover ForFreeActivities layouts
    if (m_assignedLayouts.contains(Data::Layout::FREEACTIVITIESID) && freeRunningActivities().count()>0) {
        layoutNamesToLoad << m_assignedLayouts[Data::Layout::FREEACTIVITIESID];
    }

    //! discover layouts assigned to explicit activities based on running activities
    for (const auto &activity : runningActivities()) {
        if (m_assignedLayouts.contains(activity)) {
            layoutNamesToLoad << m_assignedLayouts[activity];
        }
    }

    //! discover layouts that must be unloaded because of running activities changes
    for (const auto layout : m_centralLayouts) {
        if (!layoutNamesToLoad.contains(layout->name())) {
            layoutNamesToUnload << layout->name();
        }
    }

    for (const auto lname : preloadedLayouts) {
        if (!layoutNamesToLoad.contains(lname)) {
            layoutNamesToUnload << lname;
        }
    }

    QString defaultForcedLayout;

    //! Safety
    if (layoutNamesToLoad.isEmpty()) {
        //! If no layout is found then force loading Default Layout
        QString layoutPath = m_manager->corona()->templatesManager()->newLayout("", i18n(Templates::DEFAULTLAYOUTTEMPLATENAME));
        layoutNamesToLoad << Layout::AbstractLayout::layoutName(layoutPath);
        m_manager->setOnAllActivities(layoutNamesToLoad[0]);
        defaultForcedLayout = layoutNamesToLoad[0];
    }

    QStringList newlyActivatedLayouts;

    //! Add needed Layouts based on Activities settings
    for (const auto &layoutname : layoutNamesToLoad) {
        if (!centralLayout(layoutname)) {
            CentralLayout *newLayout = new CentralLayout(this, QString(layoutPath(layoutname)), layoutname);

            if (newLayout) {
                qDebug() << "ACTIVATING LAYOUT ::::: " << layoutname;

                //! Order of initialization steps is very important and guarantees correct startup initialization
                //! Step1: corona is set for the layout
                //! Step2: containments from the layout file are adjusted and are imported into main corona
                //! Step3: layout connects to corona signals and slots
                //! Step4: layout is added in manager and is accesible for others to find
                //! Step5: layout is attaching its initial containmens and is now considered ACTIVE
                newLayout->setCorona(m_manager->corona()); //step1
                if (!preloadedLayouts.contains(layoutname)) {
                    newLayout->importToCorona();           //step2
                }
                newLayout->initCorona();                   //step3
                addLayout(newLayout);                      //step4
                newLayout->initContainments();             //step5

                if (!defaultForcedLayout.isEmpty() && defaultForcedLayout == layoutname) {
                    emit newLayoutAdded(newLayout->data());
                }

                newlyActivatedLayouts << newLayout->name();
            }
        }
    }

    if (newlyActivatedLayouts.count()>0 && m_manager->corona()->universalSettings()->showInfoWindow()) {
        m_manager->showInfoWindow(i18np("Activating layout: <b>%2</b> ...",
                                        "Activating layouts: <b>%2</b> ...",
                                        newlyActivatedLayouts.count(),
                                        newlyActivatedLayouts.join(", ")),
                                  4000, QStringList(Data::Layout::ALLACTIVITIESID));
    }

    //! Unload no needed Layouts

    //! hide layouts that will be removed in the end
    if (!layoutNamesToUnload.isEmpty()) {
        for (const auto layoutname : layoutNamesToUnload) {
            emit currentLayoutIsSwitching(layoutname);
        }

        QTimer::singleShot(LAYOUTSINITINTERVAL, [this, layoutNamesToUnload, preloadedLayouts]() {
            unloadLayouts(layoutNamesToUnload, preloadedLayouts);
        });
    }

    currentNames.sort();
    layoutNamesToLoad.sort();

    if (currentNames != layoutNamesToLoad) {
        emit centralLayoutsChanged();
    }
}

void Synchronizer::unloadLayouts(const QStringList &layoutNames, const QStringList &preloadedLayouts)
{
    if (layoutNames.isEmpty()) {
        return;
    }

    //! Unload no needed Layouts
    for (const auto &layoutname : layoutNames) {
        CentralLayout *layout = centralLayout(layoutname);
        int posLayout = centralLayoutPos(layoutname);

        if (posLayout >= 0) {
            qDebug() << "REMOVING LAYOUT ::::: " << layoutname;
            m_centralLayouts.removeAt(posLayout);

            if (!m_manager->corona()->inQuit()) {
                layout->syncToLayoutFile(true);
            }

            layout->unloadContainments();
            layout->unloadLatteViews();
            if (!m_manager->corona()->inQuit()) {
                m_manager->clearUnloadedContainmentsFromLinkedFile(layout->unloadedContainmentsIds());
            }
            delete layout;
        } else if (preloadedLayouts.contains(layoutname)) {
            Layouts::Storage::self()->moveToLayoutFile(layoutname);
            //! just make sure that
        }
    }

    emit centralLayoutsChanged();
}

void Synchronizer::updateKWinDisabledBorders()
{
    if (KWindowSystem::isPlatformWayland()) {
        // BUG: https://bugs.kde.org/show_bug.cgi?id=428202
        // KWin::reconfigure() function blocks/freezes Latte under wayland
        return;
    }

    if (!m_manager->corona()->universalSettings()->canDisableBorders()) {
        m_manager->corona()->universalSettings()->kwin_setDisabledMaximizedBorders(false);
    } else {
        if (m_manager->corona()->layoutsManager()->memoryUsage() == MemoryUsage::SingleLayout && m_centralLayouts.size() > 0) {
            m_manager->corona()->universalSettings()->kwin_setDisabledMaximizedBorders(m_centralLayouts.at(0)->disableBordersForMaximizedWindows());
        } else if (m_manager->corona()->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
            QList<CentralLayout *> centrals = centralLayoutsForActivity(m_manager->corona()->activitiesConsumer()->currentActivity());

            for (int i = 0; i < centrals.size(); ++i) {
                CentralLayout *layout = centrals.at(i);

                if (layout->disableBordersForMaximizedWindows()) {
                    m_manager->corona()->universalSettings()->kwin_setDisabledMaximizedBorders(true);
                    return;
                }
            }

            //! avoid initialization step for example during startup that no layouts have been loaded yet
            if (centrals.size() > 0) {
                m_manager->corona()->universalSettings()->kwin_setDisabledMaximizedBorders(false);
            }

        }
    }
}

}
} // end of namespace
