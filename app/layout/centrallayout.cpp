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
    connect(this, &CentralLayout::activitiesChanged, this, &CentralLayout::saveConfig);
    connect(this, &CentralLayout::disableBordersForMaximizedWindowsChanged, this, &CentralLayout::saveConfig);
    connect(this, &CentralLayout::showInMenuChanged, this, &CentralLayout::saveConfig);
}

void CentralLayout::initToCorona(Latte::Corona *corona)
{
    if (GenericLayout::initToCorona(corona)) {
        connect(m_corona->universalSettings(), &UniversalSettings::canDisableBordersChanged, this, [&]() {
            if (m_corona->universalSettings()->canDisableBorders()) {
                kwin_setDisabledMaximizedBorders(disableBordersForMaximizedWindows());
            } else {
                kwin_setDisabledMaximizedBorders(false);
            }
        });

        if (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::SingleLayout && m_corona->universalSettings()->canDisableBorders()) {
            kwin_setDisabledMaximizedBorders(disableBordersForMaximizedWindows());
        } else if (m_corona->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts) {
            connect(m_corona->layoutsManager(), &Layouts::Manager::currentLayoutNameChanged, this, [&]() {
                if (m_corona->universalSettings()->canDisableBorders()
                    && m_corona->layoutsManager()->currentLayoutName() == name()) {
                    kwin_setDisabledMaximizedBorders(disableBordersForMaximizedWindows());
                }
            });
        }
    }
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
    kwin_setDisabledMaximizedBorders(disable);

    emit disableBordersForMaximizedWindowsChanged();
}

bool CentralLayout::kwin_disabledMaximizedBorders() const
{
    if (!m_corona) {
        return false;
    }

    return m_corona->universalSettings()->kwin_borderlessMaximizedWindowsEnabled();
}

void CentralLayout::kwin_setDisabledMaximizedBorders(bool disable)
{
    if (!m_corona) {
        return;
    }

    m_corona->universalSettings()->kwin_setDisabledMaximizedBorders(disable);
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
    cdata.isLocked = !isWritable();
    cdata.isShownInMenu = showInMenu();
    cdata.hasDisabledBorders = disableBordersForMaximizedWindows();
    cdata.activities = activities();

    return cdata;
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
}

//! OVERRIDES

const QStringList CentralLayout::appliedActivities()
{
    if (!m_corona) {
        return {};
    }

    if (isOnAllActivities() || m_corona->layoutsManager()->memoryUsage() == MemoryUsage::SingleLayout ) {
        return QStringList(Data::Layout::ALLACTIVITIESID);
    } else if (isForFreeActivities()) {
        return m_corona->layoutsManager()->synchronizer()->freeActivities();
    } else {
        return m_activities;
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
