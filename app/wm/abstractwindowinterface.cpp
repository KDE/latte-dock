/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
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

#include "abstractwindowinterface.h"

// local
#include "tracker/schemes.h"
#include "tracker/windows.h"
#include "../lattecorona.h"


namespace Latte {
namespace WindowSystem {

AbstractWindowInterface::AbstractWindowInterface(QObject *parent)
    : QObject(parent)
{
    m_corona = qobject_cast<Latte::Corona *>(parent);
    m_windowsTracker = new Tracker::Windows(this);
    m_schemesTracker = new Tracker::Schemes(this);

    rulesConfig = KSharedConfig::openConfig(QStringLiteral("taskmanagerrulesrc"));

    m_windowWaitingTimer.setInterval(150);
    m_windowWaitingTimer.setSingleShot(true);

    connect(&m_windowWaitingTimer, &QTimer::timeout, this, [&]() {
        WindowId wid = m_windowChangedWaiting;
        m_windowChangedWaiting = QVariant();
        emit windowChanged(wid);
    });

    connect(this, &AbstractWindowInterface::windowChanged, this, [&](WindowId wid) {
        qDebug() << "WINDOW CHANGED ::: " << wid;
    });
}

AbstractWindowInterface::~AbstractWindowInterface()
{
    m_windowWaitingTimer.stop();

    m_schemesTracker->deleteLater();
    m_windowsTracker->deleteLater();
}

Latte::Corona *AbstractWindowInterface::corona()
{
    return m_corona;
}

Tracker::Schemes *AbstractWindowInterface::schemesTracker()
{
    return m_schemesTracker;
}

Tracker::Windows *AbstractWindowInterface::windowsTracker() const
{
    return m_windowsTracker;
}

void AbstractWindowInterface::considerWindowChanged(WindowId wid)
{
    //! Consider if the windowChanged signal should be sent DIRECTLY or WAIT

    if (m_windowChangedWaiting == wid && m_windowWaitingTimer.isActive()) {
        //! window should be sent later
        m_windowWaitingTimer.start();
        return;
    }

    if (m_windowChangedWaiting != wid && !m_windowWaitingTimer.isActive()) {
        //! window should be sent later
        m_windowChangedWaiting = wid;
        m_windowWaitingTimer.start();
    }

    if (m_windowChangedWaiting != wid && m_windowWaitingTimer.isActive()) {
        m_windowWaitingTimer.stop();
        //! sent previous waiting window
        emit (m_windowChangedWaiting);

        //! retrigger waiting for the upcoming window
        m_windowChangedWaiting = wid;
        m_windowWaitingTimer.start();
    }
}

}
}

