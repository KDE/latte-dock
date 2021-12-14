/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "abstractwindowinterface.h"

// local
#include "tracker/schemes.h"
#include "tracker/windowstracker.h"
#include "../lattecorona.h"

// Qt
#include <QDebug>
#include <QtDBus>

// KDE
#include <KWindowSystem>
#include <KActivities/Controller>

namespace Latte {
namespace WindowSystem {

#define MAXPLASMAPANELTHICKNESS 96
#define MAXSIDEPANELTHICKNESS 512

#define KWINSERVICE "org.kde.KWin"
#define KWINVIRTUALDESKTOPMANAGERNAMESPACE "org.kde.KWin.VirtualDesktopManager"

AbstractWindowInterface::AbstractWindowInterface(QObject *parent)
    : QObject(parent),
      m_kwinServiceWatcher(new QDBusServiceWatcher(this))
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

    connect(this, &AbstractWindowInterface::windowRemoved, this, &AbstractWindowInterface::windowRemovedSlot);

    // connect(this, &AbstractWindowInterface::windowChanged, this, [&](WindowId wid) {
    //     qDebug() << "WINDOW CHANGED ::: " << wid;
    // });

    connect(m_activities.data(), &KActivities::Consumer::currentActivityChanged, this, [&](const QString &id) {
        m_currentActivity = id;
        emit currentActivityChanged();
    });

    connect(KWindowSystem::self(), &KWindowSystem::showingDesktopChanged, this, &AbstractWindowInterface::setIsShowingDesktop);

    //! KWin Service tracking
    m_kwinServiceWatcher->setConnection(QDBusConnection::sessionBus());
    m_kwinServiceWatcher->setWatchedServices(QStringList({KWINSERVICE}));
    connect(m_kwinServiceWatcher, &QDBusServiceWatcher::serviceRegistered, this, [this](const QString & serviceName) {
        if (serviceName == KWINSERVICE && !m_isKWinInterfaceAvailable) {
            initKWinInterface();
        }
    });

    connect(m_kwinServiceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, [this](const QString & serviceName) {
        if (serviceName == KWINSERVICE && m_isKWinInterfaceAvailable) {
            m_isKWinInterfaceAvailable = false;
        }
    });

    initKWinInterface();
}

AbstractWindowInterface::~AbstractWindowInterface()
{
    m_windowWaitingTimer.stop();

    m_schemesTracker->deleteLater();
    m_windowsTracker->deleteLater();
}

bool AbstractWindowInterface::isShowingDesktop() const
{
    return m_isShowingDesktop;
}

void AbstractWindowInterface::setIsShowingDesktop(const bool &showing)
{
    if (m_isShowingDesktop == showing) {
        return;
    }

    m_isShowingDesktop = showing;
    emit isShowingDesktopChanged();
}

QString AbstractWindowInterface::currentDesktop()
{
    return m_currentDesktop;
}

QString AbstractWindowInterface::currentActivity()
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

bool AbstractWindowInterface::isIgnored(const WindowId &wid) const
{
    return m_ignoredWindows.contains(wid);
}

bool AbstractWindowInterface::isFullScreenWindow(const QRect &wGeometry) const
{
    if (wGeometry.isEmpty()) {
        return false;
    }

    for (const auto scr : qGuiApp->screens()) {
        auto screenGeometry = scr->geometry();

        if (KWindowSystem::isPlatformX11() && scr->devicePixelRatio() != 1.0) {
            //!Fix for X11 Global Scale, I dont think this could be pixel perfect accurate
            auto factor = scr->devicePixelRatio();
            screenGeometry = QRect(qRound(screenGeometry.x() * factor),
                                   qRound(screenGeometry.y() * factor),
                                   qRound(screenGeometry.width() * factor),
                                   qRound(screenGeometry.height() * factor));
        }


        if (wGeometry == screenGeometry) {
            return true;
        }
    }

    return false;
}

bool AbstractWindowInterface::isPlasmaPanel(const QRect &wGeometry) const
{     
    if (wGeometry.isEmpty()) {
        return false;
    }

    bool isTouchingHorizontalEdge{false};
    bool isTouchingVerticalEdge{false};

    for (const auto scr : qGuiApp->screens()) {
        auto screenGeometry = scr->geometry();

        if (KWindowSystem::isPlatformX11() && scr->devicePixelRatio() != 1.0) {
            //!Fix for X11 Global Scale, I dont think this could be pixel perfect accurate
            auto factor = scr->devicePixelRatio();
            screenGeometry = QRect(qRound(screenGeometry.x() * factor),
                                   qRound(screenGeometry.y() * factor),
                                   qRound(screenGeometry.width() * factor),
                                   qRound(screenGeometry.height() * factor));
        }

        if (screenGeometry.contains(wGeometry.center())) {
            if (wGeometry.y() == screenGeometry.y() || wGeometry.bottom() == screenGeometry.bottom()) {
                isTouchingHorizontalEdge = true;
            }

            if (wGeometry.left() == screenGeometry.left() || wGeometry.right() == screenGeometry.right()) {
                isTouchingVerticalEdge = true;
            }

            if (isTouchingVerticalEdge && isTouchingHorizontalEdge) {
                break;
            }
        }
    }

    if ((isTouchingHorizontalEdge && wGeometry.height() < MAXPLASMAPANELTHICKNESS)
            || (isTouchingVerticalEdge && wGeometry.width() < MAXPLASMAPANELTHICKNESS)) {
        return true;
    }

    return false;
}

bool AbstractWindowInterface::isSidepanel(const QRect &wGeometry) const
{
    bool isVertical = wGeometry.height() > wGeometry.width();

    int thickness = qMin(wGeometry.width(), wGeometry.height());
    int length = qMax(wGeometry.width(), wGeometry.height());

    QRect screenGeometry;

    for (const auto scr : qGuiApp->screens()) {
        auto curScrGeometry = scr->geometry();

        if (KWindowSystem::isPlatformX11() && scr->devicePixelRatio() != 1.0) {
            //!Fix for X11 Global Scale, I dont think this could be pixel perfect accurate
            auto factor = scr->devicePixelRatio();
            curScrGeometry = QRect(qRound(curScrGeometry.x() * factor),
                                   qRound(curScrGeometry.y() * factor),
                                   qRound(curScrGeometry.width() * factor),
                                   qRound(curScrGeometry.height() * factor));
        }

        if (curScrGeometry.contains(wGeometry.center())) {
            screenGeometry = curScrGeometry;
            break;
        }
    }

    bool thicknessIsAcccepted = isVertical && ((thickness > MAXPLASMAPANELTHICKNESS) && (thickness < MAXSIDEPANELTHICKNESS));
    bool lengthIsAccepted = isVertical && !screenGeometry.isEmpty() && (length > 0.6 * screenGeometry.height());
    float sideRatio = (float)wGeometry.width() / (float)wGeometry.height();

    return (thicknessIsAcccepted && lengthIsAccepted && sideRatio<0.4);
}

bool AbstractWindowInterface::hasBlockedTracking(const WindowId &wid) const
{
    return (!isWhitelistedWindow(wid) && (isRegisteredPlasmaIgnoredWindow(wid) || isIgnored(wid)));
}

bool AbstractWindowInterface::isRegisteredPlasmaIgnoredWindow(const WindowId &wid) const
{
    return m_plasmaIgnoredWindows.contains(wid);
}

bool AbstractWindowInterface::isWhitelistedWindow(const WindowId &wid) const
{
    return m_whitelistedWindows.contains(wid);
}

bool AbstractWindowInterface::inCurrentDesktopActivity(const WindowInfoWrap &winfo)
{
    return (winfo.isValid() && winfo.isOnDesktop(currentDesktop()) && winfo.isOnActivity(currentActivity()));
}

//! KWin Interface
bool AbstractWindowInterface::isKWinRunning() const
{
    return m_isKWinInterfaceAvailable;
}

void AbstractWindowInterface::initKWinInterface()
{
    QDBusInterface kwinIface(KWINSERVICE, "/VirtualDesktopManager", KWINVIRTUALDESKTOPMANAGERNAMESPACE, QDBusConnection::sessionBus());

    if (kwinIface.isValid() && !m_isKWinInterfaceAvailable) {
        m_isKWinInterfaceAvailable = true;
        qDebug() << " KWIN SERVICE :: is available...";
        m_isVirtualDesktopNavigationWrappingAround = kwinIface.property("navigationWrappingAround").toBool();

        QDBusConnection bus = QDBusConnection::sessionBus();
        bool signalconnected = bus.connect(KWINSERVICE,
                                           "/VirtualDesktopManager",
                                           KWINVIRTUALDESKTOPMANAGERNAMESPACE,
                                           "navigationWrappingAroundChanged",
                                           this,
                                           SLOT(onVirtualDesktopNavigationWrappingAroundChanged(bool)));

        if (!signalconnected) {
            qDebug() << " KWIN SERVICE :: Virtual Desktop Manager :: navigationsWrappingSignal is not connected...";
        }
    }
}

bool AbstractWindowInterface::isVirtualDesktopNavigationWrappingAround() const
{
    return m_isVirtualDesktopNavigationWrappingAround;
}

void AbstractWindowInterface::onVirtualDesktopNavigationWrappingAroundChanged(bool navigationWrappingAround)
{
    m_isVirtualDesktopNavigationWrappingAround = navigationWrappingAround;
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

void AbstractWindowInterface::registerPlasmaIgnoredWindow(WindowId wid)
{
    if (!wid.isNull() && !m_plasmaIgnoredWindows.contains(wid)) {
        m_plasmaIgnoredWindows.append(wid);
        emit windowChanged(wid);
    }
}

void AbstractWindowInterface::unregisterPlasmaIgnoredWindow(WindowId wid)
{
    if (m_plasmaIgnoredWindows.contains(wid)) {
        m_plasmaIgnoredWindows.removeAll(wid);
    }
}

void AbstractWindowInterface::registerWhitelistedWindow(WindowId wid)
{
    if (!wid.isNull() && !m_whitelistedWindows.contains(wid)) {
        m_whitelistedWindows.append(wid);
        emit windowChanged(wid);
    }
}

void AbstractWindowInterface::unregisterWhitelistedWindow(WindowId wid)
{
    if (m_whitelistedWindows.contains(wid)) {
        m_whitelistedWindows.removeAll(wid);
    }
}

void AbstractWindowInterface::windowRemovedSlot(WindowId wid)
{
    if (m_plasmaIgnoredWindows.contains(wid)) {
        unregisterPlasmaIgnoredWindow(wid);
    }

    if (m_ignoredWindows.contains(wid)) {
        unregisterIgnoredWindow(wid);
    }

    if (m_whitelistedWindows.contains(wid)) {
        unregisterWhitelistedWindow(wid);
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

