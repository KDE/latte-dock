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

#include "layoutsettings.h"

#include <QFile>
#include <KSharedConfig>

namespace Latte {

LayoutSettings::LayoutSettings(QObject *parent, QString layoutFile, QString layoutName)
    : QObject(parent)
{
    qDebug() << "Layout file to create object: " << layoutFile << " with name: " << layoutName;

    if (QFile(layoutFile).exists()) {
        if (layoutName.isEmpty()) {
            int lastSlash = layoutFile.lastIndexOf("/");
            QString tempLayoutFile = layoutFile;
            layoutName = tempLayoutFile.remove(0, lastSlash + 1);

            int ext = layoutName.lastIndexOf(".layout.latte");
            layoutName = layoutName.remove(ext, 13);
        }

        KSharedConfigPtr lConfig = KSharedConfig::openConfig(layoutFile);
        m_layoutGroup = KConfigGroup(lConfig, "LayoutSettings");

        setFile(layoutFile);
        setName(layoutName);
        loadConfig();
        init();
    }
}

LayoutSettings::~LayoutSettings()
{
    saveConfig();
    m_layoutGroup.sync();
}

void LayoutSettings::init()
{
    connect(this, &LayoutSettings::activitiesChanged, this, &LayoutSettings::saveConfig);
    connect(this, &LayoutSettings::versionChanged, this, &LayoutSettings::saveConfig);
    connect(this, &LayoutSettings::colorChanged, this, &LayoutSettings::saveConfig);
    connect(this, &LayoutSettings::syncLaunchersChanged, this, &LayoutSettings::saveConfig);
    connect(this, &LayoutSettings::showInMenuChanged, this, &LayoutSettings::saveConfig);
    connect(this, &LayoutSettings::globalLaunchersChanged, this, &LayoutSettings::saveConfig);
}

int LayoutSettings::version() const
{
    return m_version;
}

void LayoutSettings::setVersion(int ver)
{
    if (m_version == ver) {
        return;
    }

    m_version = ver;

    emit versionChanged();
}

bool LayoutSettings::showInMenu() const
{
    return m_showInMenu;
}

void LayoutSettings::setShowInMenu(bool show)
{
    if (m_showInMenu == show) {
        return;
    }

    m_showInMenu = show;
    emit showInMenuChanged();
}

QString LayoutSettings::name() const
{
    return m_layoutName;
}

void LayoutSettings::setName(QString name)
{
    if (m_layoutName == name) {
        return;
    }

    qDebug() << "Layout name:" << name;

    m_layoutName = name;

    emit nameChanged();
}

QString LayoutSettings::color() const
{
    return m_color;
}

void LayoutSettings::setColor(QString color)
{
    if (m_color == color) {
        return;
    }

    m_color = color;
    emit colorChanged();
}


QString LayoutSettings::file() const
{
    return m_layoutFile;
}

void LayoutSettings::setFile(QString file)
{
    if (m_layoutFile == file) {
        return;
    }

    qDebug() << "Layout file:" << file;

    m_layoutFile = file;
    emit fileChanged();
}

bool LayoutSettings::syncLaunchers() const
{
    return m_syncLaunchers;
}
void LayoutSettings::setSyncLaunchers(bool sync)
{
    if (m_syncLaunchers == sync)
        return;

    m_syncLaunchers = sync;

    emit syncLaunchersChanged();
}

QStringList LayoutSettings::globalLaunchers() const
{
    return m_globalLaunchers;
}

void LayoutSettings::setGlobalLaunchers(QStringList launchers)
{
    if (m_globalLaunchers == launchers)
        return;

    m_globalLaunchers = launchers;

    emit globalLaunchersChanged();
}

QStringList LayoutSettings::activities() const
{
    return m_activities;
}

void  LayoutSettings::setActivities(QStringList activities)
{
    if (m_activities == activities) {
        return;
    }

    m_activities = activities;

    emit activitiesChanged();
}

void LayoutSettings::loadConfig()
{
    m_version = m_layoutGroup.readEntry("version", 2);
    m_color = m_layoutGroup.readEntry("color", QString("blue"));
    m_showInMenu = m_layoutGroup.readEntry("showInMenu", false);
    m_syncLaunchers = m_layoutGroup.readEntry("syncLaunchers", false);
    m_activities = m_layoutGroup.readEntry("activities", QStringList());
    m_globalLaunchers = m_layoutGroup.readEntry("globalLaunchers", QStringList());
}

void LayoutSettings::saveConfig()
{
    qDebug() << "layout is saving... for layout:" << m_layoutName;
    m_layoutGroup.writeEntry("version", m_version);
    m_layoutGroup.writeEntry("showInMenu", m_showInMenu);
    m_layoutGroup.writeEntry("color", m_color);
    m_layoutGroup.writeEntry("syncLaunchers", m_syncLaunchers);
    m_layoutGroup.writeEntry("globalLaunchers", m_globalLaunchers);
    m_layoutGroup.writeEntry("activities", m_activities);
}

}
