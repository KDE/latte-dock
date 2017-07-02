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

#include "layoutmanager.h"

namespace Latte {

LayoutManager::LayoutManager(QObject *parent)
    : QObject(parent),
      m_importer(new Importer(this))
{
    m_corona = qobject_cast<DockCorona *>(parent);
}

LayoutManager::~LayoutManager()
{
    m_importer->deleteLater();
}

void LayoutManager::load()
{
    int configVer = m_corona->universalSettings()->version();
    qDebug() << "Universal Settings version : " << configVer;

    if (configVer < 2) {
        qDebug() << "Latte must update its configuration...";
        m_importer->updateOldConfiguration();
    } else {
        qDebug() << "Latte is loading  its layouts...";
    }

}

DockCorona *LayoutManager::corona()
{
    return m_corona;
}

}
