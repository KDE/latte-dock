/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "storage.h"

// local
#include "../apptypes.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../layouts/manager.h"
#include "../layouts/importer.h"
#include "../layouts/storage.h"
#include "../view/view.h"

// Qt
#include <QDir>
#include <QFile>
#include <QFileInfo>

// KDE
#include <KConfigGroup>
#include <KSharedConfig>

// Plasma
#include <Plasma>
#include <Plasma/Applet>
#include <Plasma/Containment>

namespace Latte {
namespace Layout {

Storage::Storage(GenericLayout *parent)
    : QObject(parent),
      m_layout(parent)
{
}

Storage::~Storage()
{
}

void Storage::setStorageTmpDir(const QString &tmpDir)
{
    m_storageTmpDir = tmpDir;
}

void Storage::systraysInformation(QHash<int, QList<int>> &systrays, QList<int> &assignedSystrays, QList<int> &orphanSystrays)
{
    systrays.clear();
    assignedSystrays.clear();
    orphanSystrays.clear();

    KSharedConfigPtr lFile = KSharedConfig::openConfig(m_layout->file());
    KConfigGroup containmentGroups = KConfigGroup(lFile, "Containments");

    //! assigned systrays
    for (const auto &cId : containmentGroups.groupList()) {
        if (Layouts::Storage::self()->isLatteContainment(containmentGroups.group(cId))) {
            auto applets = containmentGroups.group(cId).group("Applets");

            for (const auto &applet : applets.groupList()) {
                KConfigGroup appletSettings = applets.group(applet).group("Configuration");
                int tSysId = appletSettings.readEntry("SystrayContainmentId", -1);

                if (tSysId != -1) {
                    assignedSystrays << tSysId;
                    systrays[cId.toInt()].append(tSysId);
                }
            }
        }
    }

    //! orphan systrays
    for (const auto &cId : containmentGroups.groupList()) {
        if (!Layouts::Storage::self()->isLatteContainment(containmentGroups.group(cId)) && !assignedSystrays.contains(cId.toInt())) {
            orphanSystrays << cId.toInt();
        }
    }
}

QList<ViewData> Storage::viewsData(const QHash<int, QList<int>> &systrays)
{
    QList<ViewData> viewsData;

    KSharedConfigPtr lFile = KSharedConfig::openConfig(m_layout->file());
    KConfigGroup containmentGroups = KConfigGroup(lFile, "Containments");

    for (const auto &cId : containmentGroups.groupList()) {
        if (Layouts::Storage::self()->isLatteContainment(containmentGroups.group(cId))) {
            ViewData vData;
            int id = cId.toInt();

            //! id
            vData.id = id;

            //! active
            vData.active = false;

            //! onPrimary
            vData.onPrimary = containmentGroups.group(cId).readEntry("onPrimary", true);

            //! Screen
            vData.screenId = containmentGroups.group(cId).readEntry("lastScreen", -1);

            //! location
            vData.location = containmentGroups.group(cId).readEntry("location", (int)Plasma::Types::BottomEdge);

            //! systrays
            vData.systrays = systrays[id];

            viewsData << vData;
        }
    }

    return viewsData;
}

QList<int> Storage::viewsScreens()
{
    QList<int> screens;

    KSharedConfigPtr lFile = KSharedConfig::openConfig(m_layout->file());

    KConfigGroup containmentGroups = KConfigGroup(lFile, "Containments");

    for (const auto &cId : containmentGroups.groupList()) {
        if (Layouts::Storage::self()->isLatteContainment(containmentGroups.group(cId))) {
            int screenId = containmentGroups.group(cId).readEntry("lastScreen", -1);

            if (screenId != -1 && !screens.contains(screenId)) {
                screens << screenId;
            }
        }
    }

    return screens;
}

}
}
