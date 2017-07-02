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

namespace Latte {

//! This class holds all the settings that are universally available
//! independent of layouts
class UniversalSettings : public QObject {
    Q_OBJECT
    //Q_PROPERTY(bool autostart READ autostart WRITE setAutostart NOTIFY autostartChanged)
    Q_PROPERTY(bool exposeLayoutsMenu READ exposeLayoutsMenu WRITE setExposeLayoutsMenu NOTIFY exposeLayoutsMenuChanged)

    //Q_PROPERTY(Latte::Dock::SessionType currentSession READ currentSession WRITE setCurrentSession NOTIFY currentSessionChanged)

    //Q_PROPERTY(QAction *altSessionAction READ altSessionAction NOTIFY altSessionActionChanged)
    //Q_PROPERTY(QAction *addWidgetsAction READ addWidgetsAction NOTIFY addWidgetsActionChanged)

public:
    UniversalSettings(KSharedConfig::Ptr config, QObject *parent = nullptr);
    ~UniversalSettings() override;

    void load();

    bool exposeLayoutsMenu() const;
    void setExposeLayoutsMenu(bool state);

    int version() const;
    void setVersion(int ver);

signals:
    void exposeLayoutsMenuChanged();
    void versionChanged();

private slots:
    void loadConfig();
    void saveConfig();

private:
    void cleanupSettings();

private:
    bool m_exposeLayoutsMenu{false};
    //when there isnt a version it is an old universal file
    int m_version{1};

    KConfigGroup m_universalGroup;
    KSharedConfig::Ptr m_config;
};

}

#endif //UNIVERSALSETTINGS_H
