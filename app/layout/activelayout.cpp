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

#include "activelayout.h"

// local
#include "../lattecorona.h"
#include "../layoutmanager.h"
#include "../settings/universalsettings.h"
#include "../../liblatte2/types.h"

// Qt
#include <QtDBus/QtDBus>

// KDE
#include <KConfigGroup>
#include <KActivities/Consumer>

namespace Latte {

ActiveLayout::ActiveLayout(QObject *parent, QString layoutFile, QString assignedName)
    : Layout::GenericLayout(parent, layoutFile, assignedName)
{
    if (m_loadedCorrectly) {
        loadConfig();
        init();
    }
}

ActiveLayout::~ActiveLayout()
{
    if (!m_layoutFile.isEmpty()) {
        m_layoutGroup.sync();
    }
}

void ActiveLayout::init()
{
    connect(this, &ActiveLayout::activitiesChanged, this, &ActiveLayout::saveConfig);
    connect(this, &ActiveLayout::disableBordersForMaximizedWindowsChanged, this, &ActiveLayout::saveConfig);
    connect(this, &ActiveLayout::showInMenuChanged, this, &ActiveLayout::saveConfig);
}

void ActiveLayout::initToCorona(Latte::Corona *corona)
{
    if (GenericLayout::initToCorona(corona)) {
        connect(m_corona->universalSettings(), &UniversalSettings::canDisableBordersChanged, this, [&]() {
            if (m_corona->universalSettings()->canDisableBorders()) {
                kwin_setDisabledMaximizedBorders(disableBordersForMaximizedWindows());
            } else {
                kwin_setDisabledMaximizedBorders(false);
            }
        });


        if (m_corona->layoutManager()->memoryUsage() == Types::SingleLayout && m_corona->universalSettings()->canDisableBorders()) {
            kwin_setDisabledMaximizedBorders(disableBordersForMaximizedWindows());
        } else if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
            connect(m_corona->layoutManager(), &LayoutManager::currentLayoutNameChanged, this, [&]() {
                if (m_corona->universalSettings()->canDisableBorders()
                    && m_corona->layoutManager()->currentLayoutName() == name()) {
                    kwin_setDisabledMaximizedBorders(disableBordersForMaximizedWindows());
                }
            });
        }

        if (m_layoutName != MultipleLayoutsName) {
            updateLastUsedActivity();
        }

        connect(m_corona->activityConsumer(), &KActivities::Consumer::currentActivityChanged,
                this, &ActiveLayout::updateLastUsedActivity);

    }
}

bool ActiveLayout::disableBordersForMaximizedWindows() const
{
    return m_disableBordersForMaximizedWindows;
}

void ActiveLayout::setDisableBordersForMaximizedWindows(bool disable)
{
    if (m_disableBordersForMaximizedWindows == disable) {
        return;
    }

    m_disableBordersForMaximizedWindows = disable;
    kwin_setDisabledMaximizedBorders(disable);

    emit disableBordersForMaximizedWindowsChanged();
}

bool ActiveLayout::kwin_disabledMaximizedBorders() const
{
    //! Identify Plasma Desktop version
    QProcess process;
    process.start("kreadconfig5 --file kwinrc --group Windows --key BorderlessMaximizedWindows");
    process.waitForFinished();
    QString output(process.readAllStandardOutput());

    output = output.remove("\n");

    return (output == "true");
}

void ActiveLayout::kwin_setDisabledMaximizedBorders(bool disable)
{
    if (kwin_disabledMaximizedBorders() == disable) {
        return;
    }

    QString disableText = disable ? "true" : "false";

    QProcess process;
    QString commandStr = "kwriteconfig5 --file kwinrc --group Windows --key BorderlessMaximizedWindows --type bool " + disableText;
    process.start(commandStr);
    process.waitForFinished();

    QDBusInterface iface("org.kde.KWin", "/KWin", "", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.call("reconfigure");
    }

}

bool ActiveLayout::showInMenu() const
{
    return m_showInMenu;
}

void ActiveLayout::setShowInMenu(bool show)
{
    if (m_showInMenu == show) {
        return;
    }

    m_showInMenu = show;
    emit showInMenuChanged();
}

QStringList ActiveLayout::activities() const
{
    return m_activities;
}

void ActiveLayout::setActivities(QStringList activities)
{
    if (m_activities == activities) {
        return;
    }

    m_activities = activities;

    emit activitiesChanged();
}

void ActiveLayout::updateLastUsedActivity()
{
    if (!m_corona) {
        return;
    }

    if (!m_lastUsedActivity.isEmpty() && !m_corona->layoutManager()->activities().contains(m_lastUsedActivity)) {
        clearLastUsedActivity();
    }

    QString currentId = m_corona->activitiesConsumer()->currentActivity();

    QStringList appliedActivitiesIds = appliedActivities();

    if (m_lastUsedActivity != currentId
        && (appliedActivitiesIds.contains(currentId)
            || m_corona->layoutManager()->memoryUsage() == Types::SingleLayout)) {
        m_lastUsedActivity = currentId;

        emit lastUsedActivityChanged();
    }
}

bool ActiveLayout::isActiveLayout() const
{
    if (!m_corona) {
        return false;
    }

    ActiveLayout *activeLayout = m_corona->layoutManager()->activeLayout(m_layoutName);

    if (activeLayout) {
        return true;
    } else {
        return false;
    }
}

bool ActiveLayout::isOriginalLayout() const
{
    return m_layoutName != MultipleLayoutsName;
}

void ActiveLayout::loadConfig()
{
    m_disableBordersForMaximizedWindows = m_layoutGroup.readEntry("disableBordersForMaximizedWindows", false);
    m_showInMenu = m_layoutGroup.readEntry("showInMenu", false);
    m_activities = m_layoutGroup.readEntry("activities", QStringList());

    emit activitiesChanged();
}

void ActiveLayout::saveConfig()
{
    qDebug() << "active layout is saving... for layout:" << m_layoutName;
    m_layoutGroup.writeEntry("showInMenu", m_showInMenu);
    m_layoutGroup.writeEntry("disableBordersForMaximizedWindows", m_disableBordersForMaximizedWindows);
    m_layoutGroup.writeEntry("activities", m_activities);

    m_layoutGroup.sync();
}

const QStringList ActiveLayout::appliedActivities()
{
    if (!m_corona) {
        return {};
    }

    if (m_corona->layoutManager()->memoryUsage() == Types::SingleLayout) {
        return {"0"};
    } else if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
        if (m_activities.isEmpty()) {
            return m_corona->layoutManager()->orphanedActivities();
        } else {
            return m_activities;
        }
    } else {
        return {"0"};
    }
}

}
