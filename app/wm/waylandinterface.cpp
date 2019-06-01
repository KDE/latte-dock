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
#include "view/screenedgeghostwindow.h"
#include "view/view.h"
#include "../lattecorona.h"
#include "../liblatte2/extras.h"

// Qt
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QSignalMapper>
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
    GhostWindow(WindowSystem::WaylandInterface *waylandInterface)
        : m_waylandInterface(waylandInterface) {
        setFlags(Qt::FramelessWindowHint
                 | Qt::WindowStaysOnTopHint
                 | Qt::NoDropShadowWindowHint
                 | Qt::WindowDoesNotAcceptFocus);

        setupWaylandIntegration();
        show();
    }

    ~GhostWindow() {
        delete m_shellSurface;
    }

    void setGeometry(const QRect &rect) {
        QWindow::setGeometry(rect);
        setMaximumSize(rect.size());
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
};

namespace WindowSystem {

WaylandInterface::WaylandInterface(QObject *parent)
    : AbstractWindowInterface(parent)
{
    m_corona = qobject_cast<Latte::Corona *>(parent);

    m_activities = new KActivities::Consumer(this);


    connect(m_activities.data(), &KActivities::Consumer::currentActivityChanged
            , this, &WaylandInterface::currentActivityChanged);
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
                if (!w || (w && (!isPlasmaDesktop(w) && w->appId() != QLatin1String("latte-dock")))) {
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

void WaylandInterface::setViewExtraFlags(QWindow &view)
{
    Q_UNUSED(view)
}

void WaylandInterface::setViewStruts(QWindow &view, const QRect &rect, Plasma::Types::Location location)
{
    if (!m_ghostWindows.contains(view.winId()))
        m_ghostWindows[view.winId()] = new Private::GhostWindow(this);

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

void WaylandInterface::setKeepAbove(const QDialog &dialog, bool above) const
{
    if (above) {
        KWindowSystem::setState(dialog.winId(), NET::KeepAbove);
    } else {
        KWindowSystem::clearState(dialog.winId(), NET::KeepAbove);
    }
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

void WaylandInterface::setEdgeStateFor(QWindow *view, bool active) const
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

    return  requestInfo(w->internalId());
}

bool WaylandInterface::isOnCurrentDesktop(WindowId wid) const
{
    if (!m_windowManagement) {
        return true;
    }

#if KF5_VERSION_MINOR >= 52
    auto window = windowFor(wid);
    if (window) {
        QStringList wvds = window->plasmaVirtualDesktops();
        return (wvds.isEmpty() || (!wvds.isEmpty() && wvds.contains(m_currentDesktop)));
    }
#endif

    return true;
}

bool WaylandInterface::isOnCurrentActivity(WindowId wid) const
{
    return true;
}

WindowInfoWrap WaylandInterface::requestInfo(WindowId wid) const
{
    WindowInfoWrap winfoWrap;

    auto w = windowFor(wid);

    if (w) {
        if (isPlasmaDesktop(w)) {
            winfoWrap.setIsValid(true);
            winfoWrap.setIsPlasmaDesktop(true);
            winfoWrap.setWid(wid);
        } else if (isValidWindow(w)) {
            winfoWrap.setIsValid(true);
            winfoWrap.setWid(wid);
            winfoWrap.setIsActive(w->isActive());
            winfoWrap.setIsMinimized(w->isMinimized());
            winfoWrap.setIsMaxVert(w->isMaximized());
            winfoWrap.setIsMaxHoriz(w->isMaximized());
            winfoWrap.setIsFullscreen(w->isFullscreen());
            winfoWrap.setIsShaded(w->isShaded());
            winfoWrap.setIsOnAllDesktops(w->isOnAllDesktops());
            winfoWrap.setGeometry(w->geometry());
            winfoWrap.setHasSkipTaskbar(w->skipTaskbar());
            winfoWrap.setDisplay(w->title());
        }
    } else {
        return {};
    }

    return winfoWrap;
}

AppData WaylandInterface::appDataFor(WindowId wid) const
{
    auto window = windowFor(wid);

    const AppData &data = appDataFromUrl(windowUrlFromMetadata(window->appId(),
                    window->pid(), rulesConfig));

    return data;
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

WindowId WaylandInterface::winIdFor(QString appId, QRect geometry) const
{
    auto it = std::find_if(m_windowManagement->windows().constBegin(), m_windowManagement->windows().constEnd(), [&appId, &geometry](PlasmaWindow * w) noexcept {
        return w->isValid() && w->appId() == appId && w->geometry() == geometry;
    });

    if (it == m_windowManagement->windows().constEnd()) {
        return 0;
    }

    return (*it)->internalId();
}


bool WaylandInterface::windowCanBeDragged(WindowId wid) const
{
    WindowInfoWrap winfo = requestInfo(wid);
    return (winfo.isValid() && !winfo.isMinimized() && !winfo.isPlasmaDesktop());
}

void WaylandInterface::releaseMouseEventFor(WindowId wid) const
{
    // this isnt really needed under wayland
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
    if (windowCanBeDragged(wid)) {
        auto w = windowFor(wid);

        if (w && isValidWindow(w)) {
            w->requestMove();
        }
    }
}

void WaylandInterface::requestToggleKeepAbove(WindowId wid) const
{
    auto w = windowFor(wid);

    if (w) {
        w->requestToggleKeepAbove();
    }
}

void WaylandInterface::requestToggleMinimized(WindowId wid) const
{
    auto w = windowFor(wid);

    if (w && isValidWindow(w)) {
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

    if (w && isValidWindow(w)) {   
#if KF5_VERSION_MINOR >= 52
        if (!m_currentDesktop.isEmpty()) {
            w->requestEnterVirtualDesktop(m_currentDesktop);
        }
#endif
        w->requestToggleMaximized();
    }   
}

bool WaylandInterface::isPlasmaDesktop(const KWayland::Client::PlasmaWindow *w) const
{
    if (!w || (w->appId() != QLatin1String("org.kde.plasmashell"))) {
        return false;
    }

    bool hasScreenGeometry{false};

    for (const auto scr : qGuiApp->screens()) {
        if (!w->geometry().isEmpty() && w->geometry() == scr->geometry()) {
            hasScreenGeometry = true;
            break;
        }
    }

    return hasScreenGeometry;
}

bool WaylandInterface::isValidWindow(const KWayland::Client::PlasmaWindow *w) const
{
    //! DEPRECATED comment is case we must reenable this
    //! because wayland does not have any way yet to identify the window type
    //! a trick is to just consider windows as valid when they can be shown in the
    //! taskbar. Of course that creates issues with plasma native dialogs
    //! e.g. widgets explorer, Activities etc. that are not used to hide
    //! the dodge views appropriately

    return w->isValid() && !isPlasmaDesktop(w) && w->appId()!=QLatin1String("latte-dock");// && !w->skipTaskbar();
}

void WaylandInterface::windowCreatedProxy(KWayland::Client::PlasmaWindow *w)
{
    if (!isValidWindow(w)) return;

    if (!mapper) mapper = new QSignalMapper(this);

    mapper->setMapping(w, w);

    connect(w, &PlasmaWindow::unmapped, this, [ &, win = w]() noexcept {
        mapper->removeMappings(win);
        emit windowRemoved(win->internalId());
    });

    connect(w, SIGNAL(activeChanged()), mapper, SLOT(map()) );
    connect(w, SIGNAL(titleChanged()), mapper, SLOT(map()) );
    connect(w, SIGNAL(fullscreenChanged()), mapper, SLOT(map()) );
    connect(w, SIGNAL(geometryChanged()), mapper, SLOT(map()) );
    connect(w, SIGNAL(maximizedChanged()), mapper, SLOT(map()) );
    connect(w, SIGNAL(minimizedChanged()), mapper, SLOT(map()) );
    connect(w, SIGNAL(shadedChanged()), mapper, SLOT(map()) );
    connect(w, SIGNAL(skipTaskbarChanged()), mapper, SLOT(map()) );
    connect(w, SIGNAL(onAllDesktopsChanged()), mapper, SLOT(map()) );
    connect(w, SIGNAL(virtualDesktopChanged()), mapper, SLOT(map()) );

#if KF5_VERSION_MINOR >= 52
    connect(w, &KWayland::Client::PlasmaWindow::plasmaVirtualDesktopEntered, this,
            [w, this] {
        mapper->map(w);
    });

    connect(w, &KWayland::Client::PlasmaWindow::plasmaVirtualDesktopLeft, this,
            [w, this] {
        mapper->map(w);
    });
#endif

    connect(mapper, static_cast<void (QSignalMapper::*)(QObject *)>(&QSignalMapper::mapped)
            , this, [&](QObject * w) noexcept {
        //qDebug() << "window changed:" << qobject_cast<PlasmaWindow *>(w)->appId();
        PlasmaWindow *pW = qobject_cast<PlasmaWindow*>(w);

        if (pW && !isPlasmaDesktop(pW) && pW->appId() != QLatin1String("latte-dock")) {
            considerWindowChanged(pW->internalId());
        }
    });

    emit windowAdded(w->internalId());
}

}
}

#include "waylandinterface.moc"
