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

/*LayoutSettings::LayoutSettings(QObject *parent, KSharedConfig::Ptr config)
    : QObject(parent)
{
    m_layoutGroup = KConfigGroup(config, "LayoutSettings");
    init();
}*/

LayoutSettings::LayoutSettings(QObject *parent, QString layoutFile, QString layoutName)
    : QObject(parent)
{
    if (QFile(layoutFile).exists()) {
        if (layoutName.isEmpty()) {
            int lastSlash = layoutFile.lastIndexOf("/");
            layoutName = layoutFile.remove(0, lastSlash + 1);

            int ext = layoutName.lastIndexOf(".latterc");
            layoutName = layoutName.remove(ext, 8);
        }

        KSharedConfigPtr lConfig = KSharedConfig::openConfig(layoutFile);
        m_layoutGroup = KConfigGroup(lConfig, "LayoutSettings");

        setFile(layoutFile);
        setName(layoutName);
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
    connect(this, &LayoutSettings::versionChanged, this, &LayoutSettings::saveConfig);
    connect(this, &LayoutSettings::syncLaunchersChanged, this, &LayoutSettings::saveConfig);
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

QString LayoutSettings::name() const
{
    return m_layoutName;
}

void LayoutSettings::setName(QString name)
{
    if (m_layoutName == name) {
        return;
    }

    m_layoutName = name;

    emit nameChanged();
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

void LayoutSettings::loadConfig()
{
    m_version = m_layoutGroup.readEntry("version", 1);
    m_syncLaunchers = m_layoutGroup.readEntry("syncLaunchers", false);
    m_globalLaunchers = m_layoutGroup.readEntry("globalLaunchers", QStringList());
}

void LayoutSettings::saveConfig()
{
    m_layoutGroup.writeEntry("version", m_version);
    m_layoutGroup.writeEntry("syncLaunchers", m_syncLaunchers);
    m_layoutGroup.writeEntry("globalLaunchers", m_globalLaunchers);
}

}
