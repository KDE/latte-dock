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

LayoutSettings::LayoutSettings(QObject *parent, KSharedConfig::Ptr config)
    : QObject(parent)
{
    m_layoutGroup = KConfigGroup(config, "LayoutSettings");
    init();
}

LayoutSettings::LayoutSettings(QObject *parent, QString layoutFile)
    : QObject(parent)
{
    if (QFile(layoutFile).exists()) {
        KSharedConfigPtr lConfig = KSharedConfig::openConfig(layoutFile);
        m_layoutGroup = KConfigGroup(lConfig, "LayoutSettings");

        m_layoutFile = layoutFile;
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

void LayoutSettings::loadConfig()
{
    m_version = m_layoutGroup.readEntry("version", 1);
}

void LayoutSettings::saveConfig()
{
    m_layoutGroup.writeEntry("version", m_version);
}


}
