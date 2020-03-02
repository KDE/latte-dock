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

#include "waylandinterface.h"

// local
#include "view/positioner.h"
#include "view/view.h"
#include "view/helpers/screenedgeghostwindow.h"
#include "../lattecorona.h"
#include "../../liblatte2/extras.h"
#include "../../liblatte2/types.h"

// Qt
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QtX11Extras/QX11Info>
#include <QRasterWindow>

// KDE
#include <KWindowSystem>
#include <KWindowInfo>
#include <KWayland/Client/surface.h>

#if KF5_VERSION_MINOR >= 52
#include <KWayland/Client/plasmavirtualdesktop.h>
#endif

// X11
#include <NETWM>

using namespace KWayland::Client;

namespace Latte {

class Private::GhostWindow : public QRasterWindow
{
    Q_OBJECT

public:
    WindowSystem::WindowId m_winId;

    GhostWindow(WindowSystem::WaylandInterface *waylandInterface)
        : m_waylandInterface(waylandInterface) {
        setFlags(Qt::FramelessWindowHint
                 | Qt::WindowStaysOnTopHint
                 | Qt::NoDropShadowWindowHint
                 | Qt::WindowDoesNotAcceptFocus);

        connect(m_waylandInterface, &WindowSystem::AbstractWindowInterface::latteWindowAdded, this, &GhostWindow::identifyWinId);

        setupWaylandIntegration();
        show();
    }

    ~GhostWindow() {
        m_waylandInterface->unregisterIgnoredWindow(m_winId);
        delete m_shellSurface;
    }

    void setGeometry(const QRect &rect) {
        if (geometry() == rect) {
            return;
        }

        m_validGeometry = rect;

        setMinimumSize(rect.size());
        setMaximumSize(rect.size());
        resize(rect.size());

        m_shellSurface->setPosition(rect.topLeft());
    }

    void setupWaylandIntegration() {
        using namespace KWayland::Client;

        if (m_shellSurface)
            return;

        Surface *s{Surface::fromWindow(this)};

        if (!s)
            return;

        m_shellSurface = m_waylandInterface->waylandCoronaInterface()->createSurface(s, this);
        qDebug() << "wayland ghost window surface was created...";

        m_shellSurface->setSkipTaskbar(true);
        m_shellSurface->setPanelTakesFocus(false);
        m_shellSurface->setRole(PlasmaShellSurface::Role::Panel);
        m_shellSurface->setPanelBehavior(PlasmaShellSurface::PanelBehavior::AlwaysVisible);
    }

    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};
    WindowSystem::WaylandInterface *m_waylandInterface{nullptr};

    //! geometry() function under wayland does not return nice results
    QRect m_validGeometry;

public slots:
    void identifyWinId() {
        if (m_winId.isNull()) {
            m_winId = m_waylandInterface->winIdFor("latte-dock", m_validGeometry);
            m_waylandInterface->registerIgnoredWindow(m_winId);
        }
    }
};

namespace WindowSystem {

WaylandInterface::WaylandInterface(QObject *parent)
    : AbstractWindowInterface(parent)
{
    m_corona = qobject_cast<Latte::Corona *>(parent);
}

WaylandInterface::~WaylandInterface()
{
}

void WaylandInterface::init()
{
}

void WaylandInterface::initWindowManagement(KWayland::Client::PlasmaWindowManagement *windowManagement)
{
    if (m_windowManagement == windowManagement) {
        return;
    }

    m_windowManagement = windowManagement;

    connect(m_windowManagement, &PlasmaWindowManagement::windowCreated, this, &WaylandInterface::windowCreatedProxy);
    connect(m_windowManagement, &PlasmaWindowManagement::activeWindowChanged, this, [&]() noexcept {
        auto w = m_windowManagement->activeWindow();
        if (!w || (w && (!m_ignoredWindows.contains(w->internalId()))) ) {
            emit activeWindowChanged(w ? w->internalId() : 0);
        }

    }, Qt::QueuedConnection);
}

#if KF5_VERSION_MINOR >= 52
void WaylandInterface::initVirtualDesktopManagement(KWayland::Client::PlasmaVirtualDesktopManagement *virtualDesktopManagement)
{
    if (m_virtualDesktopManagement == virtualDesktopManagement) {
        return;
    }

    m_virtualDesktopManagement = virtualDesktopManagement;

    connect(m_virtualDesktopManagement, &KWayland::Client::PlasmaVirtualDesktopManagement::desktopCreated, this,
            [this](const QString &id, quint32 position) {
        addDesktop(id, position);
    });

    connect(m_virtualDesktopManagement, &KWayland::Client::PlasmaVirtualDesktopManagement::desktopRemoved, this,
            [this](const QString &id) {
        m_desktops.removeAll(id);

        if (m_currentDesktop == id) {
            setCurrentDesktop(QString());
        }
    });
}

void WaylandInterface::addDesktop(const QString &id, quint32 position)
{
    if (m_desktops.contains(id)) {
        return;
    }

    m_desktops.append(id);

    const KWayland::Client::PlasmaVirtualDesktop *desktop = m_virtualDesktopManagement->getVirtualDesktop(id);

    QObject::connect(desktop, &KWayland::Client::PlasmaVirtualDesktop::activated, this,
                     [desktop, this]() {
        setCurrentDesktop(desktop->id());
    }
    );

    if (desktop->isActive()) {
        setCurrentDesktop(id);
    }
}

void WaylandInterface::setCurrentDesktop(QString desktop)
{
    if (m_currentDesktop == desktop) {
        return;
    }

    m_currentDesktop = desktop;
    emit currentDesktopChanged();
}
#endif

KWayland::Client::PlasmaShell *WaylandInterface::waylandCoronaInterface() const
{
    return m_corona->waylandCoronaInterface();
}

//! Register Latte Ignored Windows in order to NOT be tracked
void WaylandInterface::registerIgnoredWindow(WindowId wid)
{
    if (!wid.isNull() && !m_ignoredWindows.contains(wid)) {
        m_ignoredWindows.append(wid);

        KWayland::Client::PlasmaWindow *w = windowFor(wid);

        if (w) {
            untrackWindow(w);
        }

        emit windowChanged(wid);
    }
}

void WaylandInterface::unregisterIgnoredWindow(WindowId wid)
{
    if (m_ignoredWindows.contains(wid)) {
        m_ignoredWindows.removeAll(wid);
        emit windowRemoved(wid);
    }
}

void WaylandInterface::setViewExtraFlags(QObject *view, bool isPanelWindow, Latte::Types::Visibility mode)
{
    KWayland::Client::PlasmaShellSurface *surface = qobject_cast<KWayland::Client::PlasmaShellSurface *>(view);
    Latte::View *latteView = qobject_cast<Latte::View *>(view);

    if (latteView) {
        surface = latteView->surface();
    }

    if (!surface) {
        return;
    }

    surface->setSkipTaskbar(true);
#if KF5_VERSION_MINOR >= 47
    surface->setSkipSwitcher(true);
#endif

    bool atBottom{!isPanelWindow && (mode == Latte::Types::WindowsCanCover || mode == Latte::Types::WindowsAlwaysCover)};

    if (isPanelWindow) {
        surface->setRole(PlasmaShellSurface::Role::Panel);
        surface->setPanelBehavior(PlasmaShellSurface::PanelBehavior::AutoHide);
    } else {
        surface->setRole(PlasmaShellSurface::Role::Normal);
    }

    if (latteView) {
        WindowId winId = latteView->positioner()->trackedWindowId();

        auto w = windowFor(winId);
        if (w && !w->isOnAllDesktops()) {
            requestToggleIsOnAllDesktops(winId);
        }

        //! Layer to be applied
        if (mode == Latte::Types::WindowsCanCover || mode == Latte::Types::WindowsAlwaysCover) {
            setKeepBelow(winId, true);
        } else if (mode == Latte::Types::NormalWindow) {
            setKeepBelow(winId, false);
            setKeepAbove(winId, false);
        } else {
            setKeepAbove(winId, true);
        }
    }

    if (atBottom){
        //! trying to workaround WM behavior in order
        //!  1. View at the end MUST NOT HAVE FOCUSABILITY (issue example: clicking a single active task is not minimized)
        //!  2. View at the end MUST BE AT THE BOTTOM of windows stack

        QTimer::singleShot(50, [this, surface]() {
            surface->setRole(PlasmaShellSurface::Role::ToolTip);
        });

    }
}

void WaylandInterface::setViewStruts(QWindow &view, const QRect &rect, Plasma::Types::Location location)
{
    if (!m_ghostWindows.contains(view.winId())) {
        m_ghostWindows[view.winId()] = new Private::GhostWindow(this);
    }

    auto w = m_ghostWindows[view.winId()];

    switch (location) {
    case Plasma::Types::TopEdge:
    case Plasma::Types::BottomEdge:
        w->setGeometry({rect.x() + rect.width() / 2, rect.y(), 1, rect.height()});
        break;

    case Plasma::Types::LeftEdge:
    case Plasma::Types::RightEdge:
        w->setGeometry({rect.x(), rect.y() + rect.height() / 2, rect.width(), 1});
        break;

    default:
        break;
    }
}

void WaylandInterface::switchToNextVirtualDesktop() const
{
#if KF5_VERSION_MINOR >= 52
    if (!m_virtualDesktopManagement || m_desktops.count() <= 1) {
        return;
    }

    int curPos = m_desktops.indexOf(m_currentDesktop);
    int nextPos = curPos + 1;

    if (curPos == m_desktops.count()-1) {
        nextPos = 0;
    }

    KWayland::Client::PlasmaVirtualDesktop *desktopObj = m_virtualDesktopManagement->getVirtualDesktop(m_desktops[nextPos]);

    if (desktopObj) {
        desktopObj->requestActivate();
    }
#endif
}

void WaylandInterface::switchToPreviousVirtualDesktop() const
{
#if KF5_VERSION_MINOR >= 52
    if (!m_virtualDesktopManagement || m_desktops.count() <= 1) {
        return;
    }

    int curPos = m_desktops.indexOf(m_currentDesktop);
    int nextPos = curPos - 1;

    if (curPos == 0) {
        nextPos = m_desktops.count()-1;
    }

    KWayland::Client::PlasmaVirtualDesktop *desktopObj = m_virtualDesktopManagement->getVirtualDesktop(m_desktops[nextPos]);

    if (desktopObj) {
        desktopObj->requestActivate();
    }
#endif
}

void WaylandInterface::setWindowOnActivities(QWindow &window, const QStringList &activities)
{
    //! needs to updated to wayland case
    // KWindowSystem::setOnActivities(view.winId(), activities);
}

void WaylandInterface::removeViewStruts(QWindow &view) const
{
    delete m_ghostWindows.take(view.winId());
}

WindowId WaylandInterface::activeWindow() const
{
    if (!m_windowManagement) {
        return 0;
    }

    auto wid = m_windowManagement->activeWindow();

    return wid ? wid->internalId() : 0;
}

void WaylandInterface::skipTaskBar(const QDialog &dialog) const
{
    KWindowSystem::setState(dialog.winId(), NET::SkipTaskbar);
}

void WaylandInterface::slideWindow(QWindow &view, AbstractWindowInterface::Slide location) const
{
    auto slideLocation = KWindowEffects::NoEdge;

    switch (location) {
    case Slide::Top:
        slideLocation = KWindowEffects::TopEdge;
        break;

    case Slide::Bottom:
        slideLocation = KWindowEffects::BottomEdge;
        break;

    case Slide::Left:
        slideLocation = KWindowEffects::LeftEdge;
        break;

    case Slide::Right:
        slideLocation = KWindowEffects::RightEdge;
        break;

    default:
        break;
    }

    KWindowEffects::slideWindow(view.winId(), slideLocation, -1);
}

void WaylandInterface::enableBlurBehind(QWindow &view) const
{
    KWindowEffects::enableBlurBehind(view.winId());
}

void WaylandInterface::setActiveEdge(QWindow *view, bool active) const
{
    ViewPart::ScreenEdgeGhostWindow *window = qobject_cast<ViewPart::ScreenEdgeGhostWindow *>(view);

    if (!window) {
        return;
    }

    if (window->parentView()->surface() && window->parentView()->visibility()
            && (window->parentView()->visibility()->mode() == Types::DodgeActive
                || window->parentView()->visibility()->mode() == Types::DodgeMaximized
                || window->parentView()->visibility()->mode() == Types::DodgeAllWindows
                || window->parentView()->visibility()->mode() == Types::AutoHide)) {
        if (active) {
            window->showWithMask();
            window->surface()->requestHideAutoHidingPanel();
        } else {
            window->hideWithMask();
            window->surface()->requestShowAutoHidingPanel();
        }
    }
}

WindowInfoWrap WaylandInterface::requestInfoActive() const
{
    if (!m_windowManagement) {
        return {};
    }

    auto w = m_windowManagement->activeWindow();

    if (!w) return {};

    return requestInfo(w->internalId());
}

WindowInfoWrap WaylandInterface::requestInfo(WindowId wid) const
{
    WindowInfoWrap winfoWrap;

    auto w = windowFor(wid);

    if (w && isValidWindow(w)) {
        winfoWrap.setIsValid(true);
        winfoWrap.setWid(wid);
        winfoWrap.setParentId(w->parentWindow() ? w->parentWindow()->internalId() : 0);
        winfoWrap.setIsActive(w->isActive());
        winfoWrap.setIsMinimized(w->isMinimized());
        winfoWrap.setIsMaxVert(w->isMaximized());
        winfoWrap.setIsMaxHoriz(w->isMaximized());
        winfoWrap.setIsFullscreen(w->isFullscreen());
        winfoWrap.setIsShaded(w->isShaded());
        winfoWrap.setIsOnAllDesktops(w->isOnAllDesktops());
        winfoWrap.setIsOnAllActivities(true);
        winfoWrap.setIsKeepAbove(w->isKeepAbove());
        winfoWrap.setIsKeepBelow(w->isKeepBelow());
        winfoWrap.setGeometry(w->geometry());
        winfoWrap.setHasSkipTaskbar(w->skipTaskbar());

        //! BEGIN:Window Abilities
        winfoWrap.setIsClosable(w->isCloseable());
        winfoWrap.setIsFullScreenable(w->isFullscreenable());
        winfoWrap.setIsMaximizable(w->isMaximizeable());
        winfoWrap.setIsMinimizable(w->isMinimizeable());
        winfoWrap.setIsMovable(w->isMovable());
        winfoWrap.setIsResizable(w->isResizable());
        winfoWrap.setIsShadeable(w->isShadeable());
        winfoWrap.setIsVirtualDesktopsChangeable(w->isVirtualDesktopChangeable());
        //! END:Window Abilities

        winfoWrap.setDisplay(w->title());
#if KF5_VERSION_MINOR >= 52
        winfoWrap.setDesktops(w->plasmaVirtualDesktops());
#endif
        winfoWrap.setActivities(QStringList());
    } else {
        winfoWrap.setIsValid(false);
    }

    return winfoWrap;
}

AppData WaylandInterface::appDataFor(WindowId wid) const
{
    auto window = windowFor(wid);

    if (window) {
        const AppData &data = appDataFromUrl(windowUrlFromMetadata(window->appId(),
                                                                   window->pid(), rulesConfig));

        return data;
    }

    AppData empty;

    return empty;
}

KWayland::Client::PlasmaWindow *WaylandInterface::windowFor(WindowId wid) const
{
    auto it = std::find_if(m_windowManagement->windows().constBegin(), m_windowManagement->windows().constEnd(), [&wid](PlasmaWindow * w) noexcept {
            return w->isValid() && w->internalId() == wid;
});

    if (it == m_windowManagement->windows().constEnd()) {
        return nullptr;
    }

    return *it;
}

QIcon WaylandInterface::iconFor(WindowId wid) const
{
    auto window = windowFor(wid);

    if (window) {
        return window->icon();
    }


    return QIcon();
}

WindowId WaylandInterface::winIdFor(QString appId, QString title) const
{
    auto it = std::find_if(m_windowManagement->windows().constBegin(), m_windowManagement->windows().constEnd(), [&appId, &title](PlasmaWindow * w) noexcept {
        return w->isValid() && w->appId() == appId && w->title().startsWith(title);
    });

    if (it == m_windowManagement->windows().constEnd()) {
        return QVariant();
    }

    return (*it)->internalId();
}

WindowId WaylandInterface::winIdFor(QString appId, QRect geometry) const
{
    auto it = std::find_if(m_windowManagement->windows().constBegin(), m_windowManagement->windows().constEnd(), [&appId, &geometry](PlasmaWindow * w) noexcept {
        return w->isValid() && w->appId() == appId && w->geometry() == geometry;
    });

    if (it == m_windowManagement->windows().constEnd()) {
        return QVariant();
    }

    return (*it)->internalId();
}

bool WaylandInterface::windowCanBeDragged(WindowId wid) const
{
    auto w = windowFor(wid);

    if (w && isValidWindow(w)) {
        WindowInfoWrap winfo = requestInfo(wid);
        return (winfo.isValid()
                && w->isMovable()
                && !winfo.isMinimized()
                && inCurrentDesktopActivity(winfo));
    }

    return false;
}

bool WaylandInterface::windowCanBeMaximized(WindowId wid) const
{
    auto w = windowFor(wid);

    if (w && isValidWindow(w)) {
        WindowInfoWrap winfo = requestInfo(wid);
        return (winfo.isValid()
                && w->isMaximizeable()
                && !winfo.isMinimized()
                && inCurrentDesktopActivity(winfo));
    }

    return false;
}

void WaylandInterface::requestActivate(WindowId wid) const
{
    auto w = windowFor(wid);

    if (w) {
        w->requestActivate();
    }
}

void WaylandInterface::requestClose(WindowId wid) const
{
    auto w = windowFor(wid);

    if (w) {
        w->requestClose();
    }
}


void WaylandInterface::requestMoveWindow(WindowId wid, QPoint from) const
{
    WindowInfoWrap wInfo = requestInfo(wid);

    if (windowCanBeDragged(wid) && inCurrentDesktopActivity(wInfo)) {
        auto w = windowFor(wid);

        if (w && isValidWindow(w)) {
            w->requestMove();
        }
    }
}

void WaylandInterface::requestToggleIsOnAllDesktops(WindowId wid) const
{
#if KF5_VERSION_MINOR >= 52
    auto w = windowFor(wid);

    if (w && isValidWindow(w) && m_desktops.count() > 1) {
        if (w->isOnAllDesktops()) {
            w->requestEnterVirtualDesktop(m_currentDesktop);
        } else {
            const QStringList &now = w->plasmaVirtualDesktops();

            foreach (const QString &desktop, now) {
                w->requestLeaveVirtualDesktop(desktop);
            }
        }
    }
#endif
}

void WaylandInterface::requestToggleKeepAbove(WindowId wid) const
{
    auto w = windowFor(wid);

    if (w) {
        w->requestToggleKeepAbove();
    }
}

void WaylandInterface::setKeepAbove(WindowId wid, bool active) const
{
    auto w = windowFor(wid);

    if (w) {
        if (active) {
            setKeepBelow(wid, false);
        }

        if ((w->isKeepAbove() && active) || (!w->isKeepAbove() && !active)) {
            return;
        }

        w->requestToggleKeepAbove();
    }
}

void WaylandInterface::setKeepBelow(WindowId wid, bool active) const
{
    auto w = windowFor(wid);

    if (w) {
        if (active) {
            setKeepAbove(wid, false);
        }

        if ((w->isKeepBelow() && active) || (!w->isKeepBelow() && !active)) {
            return;
        }

        w->requestToggleKeepBelow();
    }
}

void WaylandInterface::requestToggleMinimized(WindowId wid) const
{
    auto w = windowFor(wid);
    WindowInfoWrap wInfo = requestInfo(wid);

    if (w && isValidWindow(w) && inCurrentDesktopActivity(wInfo)) {
#if KF5_VERSION_MINOR >= 52
        if (!m_currentDesktop.isEmpty()) {
            w->requestEnterVirtualDesktop(m_currentDesktop);
        }
#endif
        w->requestToggleMinimized();
    }
}

void WaylandInterface::requestToggleMaximized(WindowId wid) const
{
    auto w = windowFor(wid);
    WindowInfoWrap wInfo = requestInfo(wid);

    if (w && isValidWindow(w) && windowCanBeMaximized(wid) && inCurrentDesktopActivity(wInfo)) {
#if KF5_VERSION_MINOR >= 52
        if (!m_currentDesktop.isEmpty()) {
            w->requestEnterVirtualDesktop(m_currentDesktop);
        }
#endif
        w->requestToggleMaximized();
    }
}

bool WaylandInterface::isPlasmaPanel(const KWayland::Client::PlasmaWindow *w) const
{
    if (!w || (w->appId() != QLatin1String("org.kde.plasmashell"))) {
        return false;
    }

    return AbstractWindowInterface::isPlasmaPanel(w->geometry());
}

bool WaylandInterface::isFullScreenWindow(const KWayland::Client::PlasmaWindow *w) const
{
    if (!w) {
        return false;
    }

    return w->isFullscreen() || AbstractWindowInterface::isFullScreenWindow(w->geometry());
}

bool WaylandInterface::isValidWindow(const KWayland::Client::PlasmaWindow *w) const
{
    //! DEPRECATED comment is case we must reenable this
    //! because wayland does not have any way yet to identify the window type
    //! a trick is to just consider windows as valid when they can be shown in the
    //! taskbar. Of course that creates issues with plasma native dialogs
    //! e.g. widgets explorer, Activities etc. that are not used to hide
    //! the dodge views appropriately

    return w->isValid() && !m_plasmaIgnoredWindows.contains(w->internalId()) && !m_ignoredWindows.contains(w->internalId());
}

void WaylandInterface::updateWindow()
{
    PlasmaWindow *pW = qobject_cast<PlasmaWindow*>(QObject::sender());

    if (pW && !m_ignoredWindows.contains(pW->internalId())) {
        if (pW->appId() == QLatin1String("org.kde.plasmashell")) {
            if (isPlasmaPanel(pW) || isFullScreenWindow(pW)) {
                registerPlasmaIgnoredWindow(pW->internalId());
            }
        } else if ((pW->appId() == QLatin1String("latte-dock"))
                   || (pW->appId().startsWith(QLatin1String("ksmserver")))) {
            if (isFullScreenWindow(pW)) {
                registerPlasmaIgnoredWindow(pW->internalId());
            }
        }

        considerWindowChanged(pW->internalId());
    }
}

void WaylandInterface::windowUnmapped()
{
    PlasmaWindow *pW = qobject_cast<PlasmaWindow*>(QObject::sender());

    if (pW) {
        untrackWindow(pW);
        emit windowRemoved(pW->internalId());
    }
}

void WaylandInterface::trackWindow(KWayland::Client::PlasmaWindow *w)
{
    if (!w) {
        return;
    }

    connect(w, &PlasmaWindow::activeChanged, this, &WaylandInterface::updateWindow);
    connect(w, &PlasmaWindow::titleChanged, this, &WaylandInterface::updateWindow);
    connect(w, &PlasmaWindow::fullscreenChanged, this, &WaylandInterface::updateWindow);
    connect(w, &PlasmaWindow::geometryChanged, this, &WaylandInterface::updateWindow);
    connect(w, &PlasmaWindow::maximizedChanged, this, &WaylandInterface::updateWindow);
    connect(w, &PlasmaWindow::minimizedChanged, this, &WaylandInterface::updateWindow);
    connect(w, &PlasmaWindow::shadedChanged, this, &WaylandInterface::updateWindow);
    connect(w, &PlasmaWindow::skipTaskbarChanged, this, &WaylandInterface::updateWindow);
    connect(w, &PlasmaWindow::onAllDesktopsChanged, this, &WaylandInterface::updateWindow);
    connect(w, &PlasmaWindow::parentWindowChanged, this, &WaylandInterface::updateWindow);

#if KF5_VERSION_MINOR >= 52
    connect(w, &PlasmaWindow::plasmaVirtualDesktopEntered, this, &WaylandInterface::updateWindow);
    connect(w, &PlasmaWindow::plasmaVirtualDesktopLeft, this, &WaylandInterface::updateWindow);
#else
    connect(w, &PlasmaWindow::virtualDesktopChanged, this, &WaylandInterface::updateWindow);
#endif

    connect(w, &PlasmaWindow::unmapped, this, &WaylandInterface::windowUnmapped);
}

void WaylandInterface::untrackWindow(KWayland::Client::PlasmaWindow *w)
{
    if (!w) {
        return;
    }

    disconnect(w, &PlasmaWindow::activeChanged, this, &WaylandInterface::updateWindow);
    disconnect(w, &PlasmaWindow::titleChanged, this, &WaylandInterface::updateWindow);
    disconnect(w, &PlasmaWindow::fullscreenChanged, this, &WaylandInterface::updateWindow);
    disconnect(w, &PlasmaWindow::geometryChanged, this, &WaylandInterface::updateWindow);
    disconnect(w, &PlasmaWindow::maximizedChanged, this, &WaylandInterface::updateWindow);
    disconnect(w, &PlasmaWindow::minimizedChanged, this, &WaylandInterface::updateWindow);
    disconnect(w, &PlasmaWindow::shadedChanged, this, &WaylandInterface::updateWindow);
    disconnect(w, &PlasmaWindow::skipTaskbarChanged, this, &WaylandInterface::updateWindow);
    disconnect(w, &PlasmaWindow::onAllDesktopsChanged, this, &WaylandInterface::updateWindow);
    disconnect(w, &PlasmaWindow::parentWindowChanged, this, &WaylandInterface::updateWindow);

#if KF5_VERSION_MINOR >= 52
    disconnect(w, &PlasmaWindow::plasmaVirtualDesktopEntered, this, &WaylandInterface::updateWindow);
    disconnect(w, &PlasmaWindow::plasmaVirtualDesktopLeft, this, &WaylandInterface::updateWindow);
#else
    disconnect(w, &PlasmaWindow::virtualDesktopChanged, this, &WaylandInterface::updateWindow);
#endif

    disconnect(w, &PlasmaWindow::unmapped, this, &WaylandInterface::windowUnmapped);
}


void WaylandInterface::windowCreatedProxy(KWayland::Client::PlasmaWindow *w)
{
    if (!isValidWindow(w)) {
        return;
    }

    if ((w->appId() == QLatin1String("org.kde.plasmashell"))) {
        if (isPlasmaPanel(w) || isFullScreenWindow(w)) {
            registerPlasmaIgnoredWindow(w->internalId());
            return;
        }
    } else if ((w->appId() == QLatin1String("latte-dock"))
               || (w->appId().startsWith(QLatin1String("ksmserver")))) {
        if (isFullScreenWindow(w)) {
            registerPlasmaIgnoredWindow(w->internalId());
            return;
        }
    }

    trackWindow(w);
    emit windowAdded(w->internalId());

    if (w->appId() == "latte-dock") {
        emit latteWindowAdded();
    }
}

}
}

#include "waylandinterface.moc"
