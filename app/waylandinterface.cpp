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
#include "../liblattedock/extras.h"

#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QSignalMapper>
#include <QtX11Extras/QX11Info>

#include <KWindowSystem>
#include <KWindowInfo>
#include <NETWM>

using namespace KWayland::Client;

namespace Latte {

WaylandInterface::WaylandInterface(QObject *parent)
    : AbstractWindowInterface(parent)
{
    m_activities = new KActivities::Consumer(this);

    m_connection = ConnectionThread::fromApplication(this);

    if (!m_connection) {
        qWarning() << "Failed getting Wayland connection from QPA";
        return;
    }
    m_registry = new Registry(this);
    m_registry->create(m_connection);

    connect(qApp, &QCoreApplication::aboutToQuit, this, [=]() {
        if (m_wm)
            m_wm->release();

        if (m_plasmaShell)
            m_plasmaShell->release();

        m_registry->release();
    });

    m_registry->setup();
    m_connection->roundtrip();

    const auto wmInterface{m_registry->interface(Registry::Interface::PlasmaWindowManagement)};

    if (wmInterface.name == 0) {
        qWarning() << "This compositor does not support the Plasma Window Management interface";
        return;
    }

    m_wm = m_registry->createPlasmaWindowManagement(wmInterface.name, wmInterface.version, this);
    connect(m_wm, &PlasmaWindowManagement::windowCreated, this, &WaylandInterface::windowCreatedProxy);
    connect(m_wm, &PlasmaWindowManagement::activeWindowChanged, this, [&]() {
        emit activeWindowChanged(m_wm->activeWindow()->internalId());
    });


    const auto shellInterface{m_registry->interface(Registry::Interface::PlasmaShell)};

    if (shellInterface.name == 0) {
        qWarning() << "Plasma Shell interface can't be created";
        return;
    }

    m_plasmaShell = m_registry->createPlasmaShell(shellInterface.name, shellInterface.version, this);

    connect(m_activities.data(), &KActivities::Consumer::currentActivityChanged
            , this, &WaylandInterface::currentActivityChanged);

}

WaylandInterface::~WaylandInterface()
{
}

void WaylandInterface::init()
{
}

void WaylandInterface::setDockExtraFlags(QQuickWindow &view)
{
    auto surface {Surface::fromQtWinId(view.winId())};
    if (surface == nullptr) {
        qWarning() << "the surface of the Dock can't be created";
        return;
    }

    auto shellSurface {PlasmaShellSurface::get(surface)};
    if (shellSurface == nullptr) {
        qWarning() << "the shell surface can't be created";
        return;
    }

    shellSurface->setSkipTaskbar(true);
    shellSurface->setRole(PlasmaShellSurface::Role::Panel);
    shellSurface->setPanelBehavior(PlasmaShellSurface::PanelBehavior::WindowsGoBelow);
}

void WaylandInterface::setDockStruts(WindowId dockId, const QRect &dockRect
                                     , const QScreen &screen, Plasma::Types::Location location) const
{
    NETExtendedStrut strut;

    const QRect currentScreen {screen.geometry()};
    const QRect wholeScreen {{0, 0}, screen.virtualSize()};

    switch (location) {
        case Plasma::Types::TopEdge: {
            const int topOffset {screen.geometry().top()};
            strut.top_width = dockRect.height() + topOffset;
            strut.top_start = dockRect.x();
            strut.top_end = dockRect.x() + dockRect.width() - 1;
            break;
        }

        case Plasma::Types::BottomEdge: {
            const int bottomOffset {wholeScreen.bottom() - currentScreen.bottom()};
            strut.bottom_width = dockRect.height() + bottomOffset;
            strut.bottom_start = dockRect.x();
            strut.bottom_end = dockRect.x() + dockRect.width() - 1;
            break;
        }

        case Plasma::Types::LeftEdge: {
            const int leftOffset = {screen.geometry().left()};
            strut.left_width = dockRect.width() + leftOffset;
            strut.left_start = dockRect.y();
            strut.left_end = dockRect.y() + dockRect.height() - 1;
            break;
        }

        case Plasma::Types::RightEdge: {
            const int rightOffset = {wholeScreen.right() - currentScreen.right()};
            strut.right_width = dockRect.width() + rightOffset;
            strut.right_start = dockRect.y();
            strut.right_end = dockRect.y() + dockRect.height() - 1;
            break;
        }

        default:
            qWarning() << "wrong location:" << qEnumToStr(location);
            return;
    }

    KWindowSystem::setExtendedStrut(dockId.value<WId>(),
                                    strut.left_width,   strut.left_start,   strut.left_end,
                                    strut.right_width,  strut.right_start,  strut.right_end,
                                    strut.top_width,    strut.top_start,    strut.top_end,
                                    strut.bottom_width, strut.bottom_start, strut.bottom_end
                                   );
}

void WaylandInterface::removeDockStruts(WindowId dockId) const
{
    KWindowSystem::setStrut(dockId.value<WId>(), 0, 0, 0, 0);
}

WindowId WaylandInterface::activeWindow() const
{
    auto wid{m_wm->activeWindow()};

    return wid ? wid->internalId() : 0;
}

const std::list<WindowId> &WaylandInterface::windows() const
{
    return m_windows;
}

void WaylandInterface::skipTaskBar(const QDialog &dialog) const
{
    KWindowSystem::setState(dialog.winId(), NET::SkipTaskbar);
}

void WaylandInterface::slideWindow(QQuickWindow &view, AbstractWindowInterface::Slide location) const
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

void WaylandInterface::enableBlurBehind(QQuickWindow &view) const
{
    KWindowEffects::enableBlurBehind(view.winId());
}

WindowInfoWrap WaylandInterface::requestInfoActive() const
{
    auto w{m_wm->activeWindow()};

    if (!w) return {};

    WindowInfoWrap winfoWrap;
    winfoWrap.setIsValid(true);
    winfoWrap.setWid(w->internalId());
    winfoWrap.setIsActive(w->isActive());
    winfoWrap.setIsMinimized(w->isMaximized());
    winfoWrap.setIsMaxVert(w->isMaximized());
    winfoWrap.setIsMaxHoriz(w->isMaximized());
    winfoWrap.setIsFullscreen(w->isFullscreen());
    winfoWrap.setIsShaded(w->isShaded());

    return winfoWrap;
}

bool WaylandInterface::isOnCurrentDesktop(WindowId wid) const
{
    auto it = std::find_if(m_wm->windows().constBegin(), m_wm->windows().constEnd(), [&wid](PlasmaWindow * w){
        return w->isValid() && w->internalId() == wid;
    });

    return it != m_wm->windows().constEnd() && ( (*it)->virtualDesktop() == KWindowSystem::currentDesktop() || (*it)->isOnAllDesktops());
}

WindowInfoWrap WaylandInterface::requestInfo(WindowId wid) const
{
    WindowInfoWrap winfoWrap;
    auto it = std::find_if(m_wm->windows().constBegin(), m_wm->windows().constEnd(), [&wid](PlasmaWindow * w){
        return w->isValid() && w->internalId() == wid;
    });

    if (it != m_wm->windows().constEnd()) {
        auto w = *it;
        winfoWrap.setIsValid(true);
        winfoWrap.setWid(wid);
        winfoWrap.setIsActive(w->isActive());
        winfoWrap.setIsMinimized(w->isMaximized());
        winfoWrap.setIsMaxVert(w->isMaximized());
        winfoWrap.setIsMaxHoriz(w->isMaximized());
        winfoWrap.setIsFullscreen(w->isFullscreen());
        winfoWrap.setIsShaded(w->isShaded());
    } else {
        winfoWrap.setIsValid(false);
        winfoWrap.setWid(wid);
    }

    return winfoWrap;
}


inline bool WaylandInterface::isValidWindow(const KWayland::Client::PlasmaWindow *w) const
{
    return w->isValid() && !w->skipTaskbar();
}

void WaylandInterface::windowCreatedProxy(KWayland::Client::PlasmaWindow *w)
{
    if (!isValidWindow(w)) return;

    if (!mapper) mapper = new QSignalMapper(this);

    mapper->setMapping(w, w);

    connect(w, &PlasmaWindow::unmapped, this, [&, win = w]() {
        mapper->removeMappings(win);
        m_windows.remove(win->internalId());
        emit windowRemoved(win->internalId());
    });

    connect(w, SIGNAL(activeChanged()), mapper, SLOT(map()));
    connect(w, SIGNAL(fullscreenChanged()), mapper, SLOT(map()));
    connect(w, SIGNAL(geometryChanged()), mapper, SLOT(map()));
    connect(w, SIGNAL(maximizedChanged()), mapper, SLOT(map()));
    connect(w, SIGNAL(onAllDesktopsChanged()), mapper, SLOT(map()));
    connect(w, SIGNAL(shadedChanged()), mapper, SLOT(map()));
    connect(w, SIGNAL(virtualDesktopChanged()), mapper, SLOT(map()));

    connect(mapper, static_cast<void (QSignalMapper::*)(QObject *)>(&QSignalMapper::mapped)
        , this, [&](QObject *w)
    {
        emit windowChanged(qobject_cast<PlasmaWindow *>(w)->internalId());
    });

    m_windows.push_back(w->internalId());

    emit windowAdded(w->internalId());
}

}
