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
#include "tracker/trackerwindows.h"
#include "../lattecorona.h"

// Qt
#include <QDebug>

// KDE
#include <KActivities/Controller>

namespace Latte {
namespace WindowSystem {

AbstractWindowInterface::AbstractWindowInterface(QObject *parent)
    : QObject(parent)
{
    m_activities = new KActivities::Consumer(this);
    m_currentActivity = m_activities->currentActivity();

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

   // connect(this, &AbstractWindowInterface::windowChanged, this, [&](WindowId wid) {
   //     qDebug() << "WINDOW CHANGED ::: " << wid;
   // });

    connect(m_activities.data(), &KActivities::Consumer::currentActivityChanged, this, [&](const QString &id) {
        m_currentActivity = id;
        emit currentActivityChanged();
    });

}

AbstractWindowInterface::~AbstractWindowInterface()
{
    m_windowWaitingTimer.stop();

    m_schemesTracker->deleteLater();
    m_windowsTracker->deleteLater();
}

QString AbstractWindowInterface::currentDesktop() const
{
    return m_currentDesktop;
}

QString AbstractWindowInterface::currentActivity() const
{
    return m_currentActivity;
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

//! Register Latte Ignored Windows in order to NOT be tracked
void AbstractWindowInterface::registerIgnoredWindow(WindowId wid)
{
    if (!wid.isNull() && !m_ignoredWindows.contains(wid)) {
        m_ignoredWindows.append(wid);
        emit windowChanged(wid);
    }
}

void AbstractWindowInterface::unregisterIgnoredWindow(WindowId wid)
{
    if (m_ignoredWindows.contains(wid)) {
        m_ignoredWindows.removeAll(wid);
        emit windowRemoved(wid);
    }
}


//! Activities switching
void AbstractWindowInterface::switchToNextActivity()
{
    QStringList runningActivities = m_activities->activities(KActivities::Info::State::Running);
    if (runningActivities.count() <= 1) {
        return;
    }

    int curPos = runningActivities.indexOf(m_currentActivity);
    int nextPos = curPos + 1;

    if (curPos == runningActivities.count() -1) {
        nextPos = 0;
    }

    KActivities::Controller activitiesController;
    activitiesController.setCurrentActivity(runningActivities.at(nextPos));
}

void AbstractWindowInterface::switchToPreviousActivity()
{
    QStringList runningActivities = m_activities->activities(KActivities::Info::State::Running);
    if (runningActivities.count() <= 1) {
        return;
    }

    int curPos = runningActivities.indexOf(m_currentActivity);
    int nextPos = curPos - 1;

    if (curPos == 0) {
        nextPos = runningActivities.count() - 1;
    }

    KActivities::Controller activitiesController;
    activitiesController.setCurrentActivity(runningActivities.at(nextPos));
}

//! Delay window changed trigerring
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
        emit windowChanged(m_windowChangedWaiting);

        //! retrigger waiting for the upcoming window
        m_windowChangedWaiting = wid;
        m_windowWaitingTimer.start();
    }
}

}
}

