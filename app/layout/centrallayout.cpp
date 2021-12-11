/*
    SPDX-FileCopyrightText: 2017 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2017 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "centrallayout.h"

// local
#include <coretypes.h>
#include "../apptypes.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../layouts/importer.h"
#include "../layouts/manager.h"
#include "../layouts/synchronizer.h"
#include "../settings/universalsettings.h"
#include "../view/view.h"
#include "../wm/tracker/schemes.h"

// KDE
#include <KConfigGroup>
#include <KActivities/Consumer>


namespace Latte {

CentralLayout::CentralLayout(QObject *parent, QString layoutFile, QString assignedName)
    : Layout::GenericLayout(parent, layoutFile, assignedName)
{
    if (m_loadedCorrectly) {
        loadConfig();
        init();
    }
}

CentralLayout::~CentralLayout()
{
}

void CentralLayout::init()
{
    connect(this, &GenericLayout::activitiesChanged, this, &CentralLayout::saveConfig);
    connect(this, &CentralLayout::disableBordersForMaximizedWindowsChanged, this, &CentralLayout::saveConfig);
    connect(this, &CentralLayout::showInMenuChanged, this, &CentralLayout::saveConfig);
}

bool CentralLayout::initCorona()
{
    if (GenericLayout::initCorona()) {
        onSchemeFileChanged();

        connect(this, &CentralLayout::disableBordersForMaximizedWindowsChanged,
                m_corona->layoutsManager()->synchronizer(), &Layouts::Synchronizer::updateKWinDisabledBorders);

        connect(this, &Layout::AbstractLayout::schemeFileChanged, this, &CentralLayout::onSchemeFileChanged);
        connect(m_corona->wm()->schemesTracker(), &WindowSystem::Tracker::Schemes::defaultSchemeChanged, this, &CentralLayout::onSchemeFileChanged);
        return true;
    }

    return false;
}

bool CentralLayout::disableBordersForMaximizedWindows() const
{
    return m_disableBordersForMaximizedWindows;
}

void CentralLayout::setDisableBordersForMaximizedWindows(bool disable)
{
    if (m_disableBordersForMaximizedWindows == disable) {
        return;
    }

    m_disableBordersForMaximizedWindows = disable;

    emit disableBordersForMaximizedWindowsChanged();
}

bool CentralLayout::showInMenu() const
{
    return m_showInMenu;
}

void CentralLayout::setShowInMenu(bool show)
{
    if (m_showInMenu == show) {
        return;
    }

    m_showInMenu = show;
    emit showInMenuChanged();
}

bool CentralLayout::isCurrent()
{
   QStringList appliedactivities = appliedActivities();

    return (appliedactivities.contains(Data::Layout::ALLACTIVITIESID)
            || appliedactivities.contains(m_corona->activitiesConsumer()->currentActivity()));
}

bool CentralLayout::isOnAllActivities() const
{
    return (m_activities.count() == 1 && m_activities[0] == Data::Layout::ALLACTIVITIESID);
}

bool CentralLayout::isForFreeActivities() const
{
    return (m_activities.count() == 1 && m_activities[0] == Data::Layout::FREEACTIVITIESID);
}

Layout::Type CentralLayout::type() const
{
    return Layout::Type::Central;
}

QStringList CentralLayout::activities() const
{
    return m_activities;
}

void CentralLayout::setActivities(QStringList activities)
{
    if (m_activities == activities) {
        return;
    }

    m_activities = activities;

    emit activitiesChanged();
}

Latte::WindowSystem::SchemeColors *CentralLayout::scheme() const
{
    return m_scheme;
}

void CentralLayout::setScheme(Latte::WindowSystem::SchemeColors *_scheme)
{
    if (m_scheme == _scheme) {
        return;
    }

    m_scheme = _scheme;
    emit schemeChanged();
}

Data::Layout CentralLayout::data() const
{
    Data::Layout cdata;

    cdata.id = file();
    cdata.name = name();
    cdata.icon = icon();
    cdata.backgroundStyle = backgroundStyle();
    cdata.color = color();
    cdata.background = customBackground();
    cdata.textColor = customTextColor();
    cdata.isActive = (m_corona != nullptr);
    cdata.isLocked = !isWritable();
    cdata.isShownInMenu = showInMenu();
    cdata.hasDisabledBorders = disableBordersForMaximizedWindows();
    cdata.popUpMargin = popUpMargin();
    cdata.schemeFile = schemeFile();
    cdata.activities = activities();
    cdata.lastUsedActivity = lastUsedActivity();

    cdata.errors = errors().count();
    cdata.warnings = warnings().count();

    return cdata;
}

void CentralLayout::onSchemeFileChanged()
{
    setScheme(m_corona->wm()->schemesTracker()->schemeForFile(schemeFile()));
}

void CentralLayout::loadConfig()
{
    m_disableBordersForMaximizedWindows = m_layoutGroup.readEntry("disableBordersForMaximizedWindows", false);
    m_showInMenu = m_layoutGroup.readEntry("showInMenu", false);     
    m_activities = m_layoutGroup.readEntry("activities", QStringList());

    emit activitiesChanged();
}

void CentralLayout::saveConfig()
{
    qDebug() << "CENTRAL layout is saving... for layout:" << m_layoutName;
    m_layoutGroup.writeEntry("showInMenu", m_showInMenu);
    m_layoutGroup.writeEntry("disableBordersForMaximizedWindows", m_disableBordersForMaximizedWindows);
    m_layoutGroup.writeEntry("activities", m_activities);
    m_layoutGroup.sync();
}

//! OVERRIDES

const QStringList CentralLayout::appliedActivities()
{
    if (!m_corona) {
        return {};
    }

    if (isOnAllActivities() || m_corona->layoutsManager()->memoryUsage() == MemoryUsage::SingleLayout) {
        return QStringList(Data::Layout::ALLACTIVITIESID);
    } else if (isForFreeActivities()) {
        return m_corona->layoutsManager()->synchronizer()->freeRunningActivities();
    } else {
        return m_corona->layoutsManager()->synchronizer()->validActivities(m_activities);
    }
}

Types::ViewType CentralLayout::latteViewType(uint containmentId) const
{
    for (const auto view : m_latteViews) {
        if (view->containment() && view->containment()->id() == (uint)containmentId) {
            return view->type();
        }
    }

    return Types::DockView;
}

}
