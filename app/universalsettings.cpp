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

#include "universalsettings.h"
#include "dockcorona.h"

#include "sortedactivitiesmodel.h"

#include <QDir>

#include <KActivities/Consumer>

namespace Latte {

UniversalSettings::UniversalSettings(KSharedConfig::Ptr config, QObject *parent)
    : QObject(parent),
      m_config(config),
      m_universalGroup(KConfigGroup(config, QStringLiteral("UniversalSettings")))
{
    connect(this, &UniversalSettings::canDisableBordersChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::currentLayoutNameChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::downloadWindowSizeChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::lastNonAssignedLayoutNameChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::launchersChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::layoutsColumnWidthsChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::layoutsMemoryUsageChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::layoutsWindowSizeChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::mouseSensitivityChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::screenTrackerIntervalChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::showInfoWindowChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::versionChanged, this, &UniversalSettings::saveConfig);
}

UniversalSettings::~UniversalSettings()
{
    saveConfig();
    cleanupSettings();

    if (m_runningActivitiesModel) {
        m_runningActivitiesModel->deleteLater();
    }
}

void UniversalSettings::load()
{
    //! check if user has set the autostart option
    bool autostartUserSet = m_universalGroup.readEntry("userConfiguredAutostart", false);

    if (!autostartUserSet && !autostart()) {
        setAutostart(true);
    }

    //! load configuration
    loadConfig();
}

bool UniversalSettings::showInfoWindow() const
{
    return m_showInfoWindow;
}

void UniversalSettings::setShowInfoWindow(bool show)
{
    if (m_showInfoWindow == show) {
        return;
    }

    m_showInfoWindow = show;
    emit showInfoWindowChanged();
}

int UniversalSettings::version() const
{
    return m_version;
}

void UniversalSettings::setVersion(int ver)
{
    if (m_version == ver) {
        return;
    }

    m_version = ver;

    emit versionChanged();
}

int UniversalSettings::screenTrackerInterval() const
{
    return m_screenTrackerInterval;
}

void UniversalSettings::setScreenTrackerInterval(int duration)
{
    if (m_screenTrackerInterval == duration) {
        return;
    }

    m_screenTrackerInterval = duration;
    emit screenTrackerIntervalChanged();
}

QString UniversalSettings::currentLayoutName() const
{
    return m_currentLayoutName;
}

void UniversalSettings::setCurrentLayoutName(QString layoutName)
{
    if (m_currentLayoutName == layoutName) {
        return;
    }

    m_currentLayoutName = layoutName;
    emit currentLayoutNameChanged();
}

QString UniversalSettings::lastNonAssignedLayoutName() const
{
    return m_lastNonAssignedLayoutName;
}

void UniversalSettings::setLastNonAssignedLayoutName(QString layoutName)
{
    if (m_lastNonAssignedLayoutName == layoutName) {
        return;
    }

    m_lastNonAssignedLayoutName = layoutName;
    emit lastNonAssignedLayoutNameChanged();
}

QSize UniversalSettings::downloadWindowSize() const
{
    return m_downloadWindowSize;
}

void UniversalSettings::setDownloadWindowSize(QSize size)
{
    if (m_downloadWindowSize == size) {
        return;
    }

    m_downloadWindowSize = size;
    emit downloadWindowSizeChanged();
}


QSize UniversalSettings::layoutsWindowSize() const
{
    return m_layoutsWindowSize;
}

void UniversalSettings::setLayoutsWindowSize(QSize size)
{
    if (m_layoutsWindowSize == size) {
        return;
    }

    m_layoutsWindowSize = size;
    emit layoutsWindowSizeChanged();
}

QStringList UniversalSettings::layoutsColumnWidths() const
{
    return m_layoutsColumnWidths;
}

void UniversalSettings::setLayoutsColumnWidths(QStringList widths)
{
    if (m_layoutsColumnWidths == widths) {
        return;
    }

    m_layoutsColumnWidths = widths;
    emit layoutsColumnWidthsChanged();
}

QStringList UniversalSettings::launchers() const
{
    return m_launchers;
}

void UniversalSettings::setLaunchers(QStringList launcherList)
{
    if (m_launchers == launcherList) {
        return;
    }

    m_launchers = launcherList;
    emit launchersChanged();
}


bool UniversalSettings::autostart() const
{
    QFile autostartFile(QDir::homePath() + "/.config/autostart/org.kde.latte-dock.desktop");
    return autostartFile.exists();
}

void UniversalSettings::setAutostart(bool state)
{
    //! remove old autostart file
    QFile oldAutostartFile(QDir::homePath() + "/.config/autostart/latte-dock.desktop");

    if (oldAutostartFile.exists()) {
        oldAutostartFile.remove();
    }

    //! end of removal of old autostart file

    QFile autostartFile(QDir::homePath() + "/.config/autostart/org.kde.latte-dock.desktop");
    QFile metaFile("/usr/share/applications/org.kde.latte-dock.desktop");

    if (!state && autostartFile.exists()) {
        //! the first time that the user disables the autostart, this is recorded
        //! and from now own it will not be recreated it in the beginning
        if (!m_universalGroup.readEntry("userConfiguredAutostart", false)) {
            m_universalGroup.writeEntry("userConfiguredAutostart", true);
        }

        autostartFile.remove();
        emit autostartChanged();
    } else if (state && metaFile.exists()) {
        metaFile.copy(autostartFile.fileName());
        //! I havent added the flag "OnlyShowIn=KDE;" into the autostart file
        //! because I fall onto a Plasma 5.8 case that this flag
        //! didnt let the plasma desktop to start
        emit autostartChanged();
    }
}

bool UniversalSettings::canDisableBorders() const
{
    return m_canDisableBorders;
}

void UniversalSettings::setCanDisableBorders(bool enable)
{
    if (m_canDisableBorders == enable) {
        return;
    }

    m_canDisableBorders = enable;
    emit canDisableBordersChanged();
}

Dock::LayoutsMemoryUsage UniversalSettings::layoutsMemoryUsage() const
{
    return m_memoryUsage;
}

void UniversalSettings::setLayoutsMemoryUsage(Dock::LayoutsMemoryUsage layoutsMemoryUsage)
{
    if (m_memoryUsage == layoutsMemoryUsage) {
        return;
    }

    m_memoryUsage = layoutsMemoryUsage;
    emit layoutsMemoryUsageChanged();
}

Dock::MouseSensitivity UniversalSettings::mouseSensitivity() const
{
    return m_mouseSensitivity;
}

void UniversalSettings::setMouseSensitivity(Dock::MouseSensitivity sensitivity)
{
    if (m_mouseSensitivity == sensitivity) {
        return;
    }

    m_mouseSensitivity = sensitivity;
    emit mouseSensitivityChanged();
}

void UniversalSettings::loadConfig()
{
    m_version = m_universalGroup.readEntry("version", 1);
    m_canDisableBorders = m_universalGroup.readEntry("canDisableBorders", false);
    m_currentLayoutName = m_universalGroup.readEntry("currentLayout", QString());
    m_downloadWindowSize = m_universalGroup.readEntry("downloadWindowSize", QSize(800, 550));
    m_lastNonAssignedLayoutName = m_universalGroup.readEntry("lastNonAssignedLayout", QString());
    m_layoutsWindowSize = m_universalGroup.readEntry("layoutsWindowSize", QSize(700, 450));
    m_layoutsColumnWidths = m_universalGroup.readEntry("layoutsColumnWidths", QStringList());
    m_launchers = m_universalGroup.readEntry("launchers", QStringList());
    m_screenTrackerInterval = m_universalGroup.readEntry("screenTrackerInterval", 2500);
    m_showInfoWindow = m_universalGroup.readEntry("showInfoWindow", true);
    m_memoryUsage = static_cast<Dock::LayoutsMemoryUsage>(m_universalGroup.readEntry("memoryUsage", (int)Dock::SingleLayout));
    m_mouseSensitivity = static_cast<Dock::MouseSensitivity>(m_universalGroup.readEntry("mouseSensitivity", (int)Dock::HighSensitivity));
}

void UniversalSettings::saveConfig()
{
    m_universalGroup.writeEntry("version", m_version);
    m_universalGroup.writeEntry("canDisableBorders", m_canDisableBorders);
    m_universalGroup.writeEntry("currentLayout", m_currentLayoutName);
    m_universalGroup.writeEntry("downloadWindowSize", m_downloadWindowSize);
    m_universalGroup.writeEntry("lastNonAssignedLayout", m_lastNonAssignedLayoutName);
    m_universalGroup.writeEntry("layoutsWindowSize", m_layoutsWindowSize);
    m_universalGroup.writeEntry("layoutsColumnWidths", m_layoutsColumnWidths);
    m_universalGroup.writeEntry("launchers", m_launchers);
    m_universalGroup.writeEntry("screenTrackerInterval", m_screenTrackerInterval);
    m_universalGroup.writeEntry("showInfoWindow", m_showInfoWindow);
    m_universalGroup.writeEntry("memoryUsage", (int)m_memoryUsage);
    m_universalGroup.writeEntry("mouseSensitivity", (int)m_mouseSensitivity);

    m_universalGroup.sync();
}

void UniversalSettings::cleanupSettings()
{
    KConfigGroup containments = KConfigGroup(m_config, QStringLiteral("Containments"));
    containments.deleteGroup();

    containments.sync();
}

QString UniversalSettings::splitterIconPath()
{
    auto *dockCorona = qobject_cast<DockCorona *>(parent());

    if (dockCorona) {
        return dockCorona->kPackage().filePath("splitter");
    }

    return "";
}

QString UniversalSettings::trademarkIconPath()
{
    auto *dockCorona = qobject_cast<DockCorona *>(parent());

    if (dockCorona) {
        return dockCorona->kPackage().filePath("trademark");
    }

    return "";
}

QAbstractItemModel *UniversalSettings::runningActivitiesModel() const
{
    return m_runningActivitiesModel;
}

void UniversalSettings::setRunningActivitiesModel(SortedActivitiesModel *model)
{
    if (m_runningActivitiesModel == model) {
        return;
    }

    if (m_runningActivitiesModel) {
        m_runningActivitiesModel->deleteLater();
    }

    m_runningActivitiesModel = model;

    emit runningActivitiesModelChanged();
}

void UniversalSettings::enableActivitiesModel()
{
    if (!m_runningActivitiesModel) {
        setRunningActivitiesModel(new SortedActivitiesModel({KActivities::Info::Running, KActivities::Info::Stopping}, this));
    }
}

void UniversalSettings::disableActivitiesModel()
{
    if (m_runningActivitiesModel) {
        setRunningActivitiesModel(nullptr);
    }
}

float UniversalSettings::luminasFromFile(QString imageFile, int edge)
{
    enableActivitiesModel();

    return m_runningActivitiesModel->luminasFromFile(imageFile, edge);
}

}
