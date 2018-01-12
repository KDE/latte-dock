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

#ifndef UNIVERSALSETTINGS_H
#define UNIVERSALSETTINGS_H

#include <QObject>

#include <KConfigGroup>
#include <KSharedConfig>

#include "../liblattedock/dock.h"

namespace Latte {

class LayoutManager;

//! This class holds all the settings that are universally available
//! independent of layouts
class UniversalSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool autostart READ autostart WRITE setAutostart NOTIFY autostartChanged)
    Q_PROPERTY(bool showInfoWindow READ showInfoWindow WRITE setShowInfoWindow NOTIFY showInfoWindowChanged)

    Q_PROPERTY(QString currentLayoutName READ currentLayoutName WRITE setCurrentLayoutName NOTIFY currentLayoutNameChanged)

    Q_PROPERTY(QStringList launchers READ launchers WRITE setLaunchers NOTIFY launchersChanged)
public:
    UniversalSettings(KSharedConfig::Ptr config, QObject *parent = nullptr);
    ~UniversalSettings() override;

    void load();

    bool autostart() const;
    void setAutostart(bool state);

    bool showInfoWindow() const;
    void setShowInfoWindow(bool show);

    int version() const;
    void setVersion(int ver);

    QString currentLayoutName() const;
    void setCurrentLayoutName(QString layoutName);

    QString lastNonAssignedLayoutName() const;
    void setLastNonAssignedLayoutName(QString layoutName);

    QSize layoutsWindowSize() const;
    void setLayoutsWindowSize(QSize);

    QStringList launchers() const;
    void setLaunchers(QStringList launcherList);

signals:
    void autostartChanged();
    void currentLayoutNameChanged();
    void lastNonAssignedLayoutNameChanged();
    void layoutsWindowSizeChanged();
    void launchersChanged();
    void layoutsMemoryUsageChanged();
    void showInfoWindowChanged();
    void versionChanged();

private slots:
    void loadConfig();
    void saveConfig();

private:
    void cleanupSettings();

    Dock::LayoutsMemoryUsage layoutsMemoryUsage() const;
    void setLayoutsMemoryUsage(Dock::LayoutsMemoryUsage layoutsMemoryUsage);

private:
    bool m_showInfoWindow{true};

    //when there isnt a version it is an old universal file
    int m_version{1};

    QString m_currentLayoutName;
    QString m_lastNonAssignedLayoutName;
    QSize m_layoutsWindowSize{700, 450};
    QStringList m_launchers;
    Dock::LayoutsMemoryUsage m_memoryUsage;

    KConfigGroup m_universalGroup;
    KSharedConfig::Ptr m_config;

    friend class LayoutManager;
};

}

#endif //UNIVERSALSETTINGS_H
