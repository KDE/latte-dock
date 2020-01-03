/*
*  Copyright 2018 Michail Vourlakos <mvourlakos@gmail.com>
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

#include "screenedgeghostwindow.h"

// local
#include "view.h"

// Qt
#include <QDebug>
#include <QSurfaceFormat>
#include <QQuickView>
#include <QTimer>

// KDE
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem>

// X11
#include <NETWM>

namespace Latte {
namespace ViewPart {

ScreenEdgeGhostWindow::ScreenEdgeGhostWindow(Latte::View *view) :
    m_latteView(view)
{
    m_corona = qobject_cast<Latte::Corona *>(view->corona());

    bool debugEdge = (qApp->arguments().contains("-d") && qApp->arguments().contains("--kwinedges"));

    setColor(debugEdge ? QColor("purple") : QColor(Qt::transparent));
    setDefaultAlphaBuffer(true);

    setFlags(Qt::FramelessWindowHint
             | Qt::WindowStaysOnTopHint
             | Qt::NoDropShadowWindowHint
             | Qt::WindowDoesNotAcceptFocus);

    m_fixGeometryTimer.setSingleShot(true);
    m_fixGeometryTimer.setInterval(500);
    connect(&m_fixGeometryTimer, &QTimer::timeout, this, &ScreenEdgeGhostWindow::fixGeometry);

    //! this timer is used in order to avoid fast enter/exit signals during first
    //! appearing after edge activation
    m_delayedMouseTimer.setSingleShot(true);
    m_delayedMouseTimer.setInterval(50);
    connect(&m_delayedMouseTimer, &QTimer::timeout, this, [this]() {
        if (m_delayedContainsMouse) {
            setContainsMouse(true);
        } else {
            setContainsMouse(false);
        }
    });

    connect(this, &QQuickView::xChanged, this, &ScreenEdgeGhostWindow::startGeometryTimer);
    connect(this, &QQuickView::yChanged, this, &ScreenEdgeGhostWindow::startGeometryTimer);
    connect(this, &QQuickView::widthChanged, this, &ScreenEdgeGhostWindow::startGeometryTimer);
    connect(this, &QQuickView::heightChanged, this, &ScreenEdgeGhostWindow::startGeometryTimer);

    connect(m_latteView, &Latte::View::absoluteGeometryChanged, this, &ScreenEdgeGhostWindow::updateGeometry);
    connect(m_latteView, &Latte::View::screenGeometryChanged, this, &ScreenEdgeGhostWindow::updateGeometry);
    connect(m_latteView, &Latte::View::locationChanged, this, &ScreenEdgeGhostWindow::updateGeometry);
    connect(m_latteView, &QQuickView::screenChanged, this, [this]() {
        setScreen(m_latteView->screen());
        updateGeometry();
    });

    if (!KWindowSystem::isPlatformWayland()) {
        //! IMPORTANT!!! ::: This fixes a bug when closing an Activity all views from all Activities are
        //!  disappearing! With this code parts they reappear!!!
        m_visibleHackTimer1.setInterval(400);
        m_visibleHackTimer2.setInterval(2500);
        m_visibleHackTimer1.setSingleShot(true);
        m_visibleHackTimer2.setSingleShot(true);

        connectionsHack << connect(this, &QWindow::visibleChanged, this, [&]() {
            if (!m_inDelete && m_latteView && m_latteView->layout() && !isVisible()) {
                m_visibleHackTimer1.start();
                m_visibleHackTimer2.start();
            } else if (!m_inDelete) {
                //! For some reason when the window is hidden in the edge under X11 afterwards
                //! is losing its window flags
                m_corona->wm()->setViewExtraFlags(this);
            }
        });

        connectionsHack << connect(&m_visibleHackTimer1, &QTimer::timeout, this, [&]() {
            if (!m_inDelete && m_latteView && m_latteView->layout() && !isVisible()) {
                show();
                emit forcedShown();
                //qDebug() << "Ghost Edge:: Enforce reshow from timer 1...";
            } else {
                //qDebug() << "Ghost Edge:: No needed reshow from timer 1...";
            }
        });

        connectionsHack << connect(&m_visibleHackTimer2, &QTimer::timeout, this, [&]() {
            if (!m_inDelete && m_latteView && m_latteView->layout() && !isVisible()) {
                show();
                emit forcedShown();
                //qDebug() << "Ghost Edge:: Enforce reshow from timer 2...";
            } else {
                //qDebug() << "Ghost Edge:: No needed reshow from timer 2...";
            }
        });

        connectionsHack << connect(this, &ScreenEdgeGhostWindow::forcedShown, this, [&]() {
            m_corona->wm()->unregisterIgnoredWindow(m_trackedWindowId);
            m_trackedWindowId = winId();
            m_corona->wm()->registerIgnoredWindow(m_trackedWindowId);
        });
    }

    setupWaylandIntegration();

    if (KWindowSystem::isPlatformX11()) {
        m_trackedWindowId = winId();
        m_corona->wm()->registerIgnoredWindow(m_trackedWindowId);
    } else {
        connect(m_corona->wm(), &WindowSystem::AbstractWindowInterface::latteWindowAdded, this, [&]() {
            if (m_trackedWindowId.isNull()) {
                m_trackedWindowId = m_corona->wm()->winIdFor("latte-dock", geometry());
                m_corona->wm()->registerIgnoredWindow(m_trackedWindowId);
            }
        });
    }

    setScreen(m_latteView->screen());
    show();
    updateGeometry();
    hideWithMask();
}

ScreenEdgeGhostWindow::~ScreenEdgeGhostWindow()
{
    m_inDelete = true;

    m_corona->wm()->unregisterIgnoredWindow(KWindowSystem::isPlatformX11() ? winId() : m_trackedWindowId);

    m_latteView = nullptr;

    // clear mode
    m_visibleHackTimer1.stop();
    m_visibleHackTimer2.stop();
    for (auto &c : connectionsHack) {
        disconnect(c);
    }

    if (m_shellSurface) {
        delete m_shellSurface;
    }
}

int ScreenEdgeGhostWindow::location()
{
    return (int)m_latteView->location();
}

int ScreenEdgeGhostWindow::thickness() const
{
    return m_thickness;
}

Latte::View *ScreenEdgeGhostWindow::parentView()
{
    return m_latteView;
}

KWayland::Client::PlasmaShellSurface *ScreenEdgeGhostWindow::surface()
{
    return m_shellSurface;
}

void ScreenEdgeGhostWindow::updateGeometry()
{
    if (m_latteView->positioner()->slideOffset() != 0) {
        return;
    }

    QRect newGeometry;

    if (KWindowSystem::compositingActive()) {
        m_thickness = 6;
    } else {
        m_thickness = 2;
    }

    int length{30};
    int lengthDifference{0};

    if (m_latteView->formFactor() == Plasma::Types::Horizontal) {
        //! set minimum length to be 25% of screen width
        length = qMax(m_latteView->screenGeometry().width()/4,qMin(m_latteView->absoluteGeometry().width(), m_latteView->screenGeometry().width() - 1));
        lengthDifference = qMax(0,length - m_latteView->absoluteGeometry().width());
    } else {
        //! set minimum length to be 25% of screen height
        length = qMax(m_latteView->screenGeometry().height()/4,qMin(m_latteView->absoluteGeometry().height(), m_latteView->screenGeometry().height() - 1));
        lengthDifference = qMax(0,length - m_latteView->absoluteGeometry().height());
    }

    if (m_latteView->location() == Plasma::Types::BottomEdge) {
        int xF = qMax(m_latteView->screenGeometry().left(), m_latteView->absoluteGeometry().left() - lengthDifference);
        newGeometry.setX(xF);
        newGeometry.setY(m_latteView->screenGeometry().bottom() - m_thickness);
    } else if (m_latteView->location() == Plasma::Types::TopEdge) {
        int xF = qMax(m_latteView->screenGeometry().left(), m_latteView->absoluteGeometry().left() - lengthDifference);
        newGeometry.setX(xF);
        newGeometry.setY(m_latteView->screenGeometry().top());
    } else if (m_latteView->location() == Plasma::Types::LeftEdge) {
        int yF = qMax(m_latteView->screenGeometry().top(), m_latteView->absoluteGeometry().top() - lengthDifference);
        newGeometry.setX(m_latteView->screenGeometry().left());
        newGeometry.setY(yF);
    } else if (m_latteView->location() == Plasma::Types::RightEdge) {
        int yF = qMax(m_latteView->screenGeometry().top(), m_latteView->absoluteGeometry().top() - lengthDifference);
        newGeometry.setX(m_latteView->screenGeometry().right() - m_thickness);
        newGeometry.setY(yF);
    }

    if (m_latteView->formFactor() == Plasma::Types::Horizontal) {
        newGeometry.setWidth(length);
        newGeometry.setHeight(m_thickness + 1);
    } else {
        newGeometry.setWidth(m_thickness + 1);
        newGeometry.setHeight(length);
    }

    m_calculatedGeometry = newGeometry;

    fixGeometry();
}

void ScreenEdgeGhostWindow::fixGeometry()
{
    if (!m_calculatedGeometry.isEmpty()
            && (m_calculatedGeometry.x() != x() || m_calculatedGeometry.y() != y()
                || m_calculatedGeometry.width() != width() || m_calculatedGeometry.height() != height())) {
        setMinimumSize(m_calculatedGeometry.size());
        setMaximumSize(m_calculatedGeometry.size());
        resize(m_calculatedGeometry.size());
        setPosition(m_calculatedGeometry.x(), m_calculatedGeometry.y());

        if (m_shellSurface) {
            m_shellSurface->setPosition(m_calculatedGeometry.topLeft());
        }
    }
}

void ScreenEdgeGhostWindow::startGeometryTimer()
{
    m_fixGeometryTimer.start();
}

void ScreenEdgeGhostWindow::setupWaylandIntegration()
{
    if (m_shellSurface || !KWindowSystem::isPlatformWayland() || !m_latteView || !m_latteView->containment()) {
        // already setup
        return;
    }

    if (m_corona) {
        using namespace KWayland::Client;

        PlasmaShell *interface = m_corona->waylandCoronaInterface();

        if (!interface) {
            return;
        }

        Surface *s = Surface::fromWindow(this);

        if (!s) {
            return;
        }

        qDebug() << "wayland screen edge ghost window surface was created...";
        m_shellSurface = interface->createSurface(s, this);
        m_corona->wm()->setViewExtraFlags(m_shellSurface);

        m_shellSurface->setPanelTakesFocus(false);
    }
}

bool ScreenEdgeGhostWindow::containsMouse() const
{
    return m_containsMouse;
}

void ScreenEdgeGhostWindow::setContainsMouse(bool contains)
{
    if (m_containsMouse == contains) {
        return;
    }

    m_containsMouse = contains;
    emit containsMouseChanged(contains);
}

bool ScreenEdgeGhostWindow::event(QEvent *e)
{
    if (e->type() == QEvent::DragEnter || e->type() == QEvent::DragMove) {
        if (!m_containsMouse) {
            m_delayedContainsMouse = false;
            m_delayedMouseTimer.stop();
            setContainsMouse(true);
            emit dragEntered();
        }
    } else if (e->type() == QEvent::Enter) {
        m_delayedContainsMouse = true;
        if (!m_delayedMouseTimer.isActive()) {
            m_delayedMouseTimer.start();
        }
    } else if (e->type() == QEvent::Leave || e->type() == QEvent::DragLeave) {
        m_delayedContainsMouse = false;
        if (!m_delayedMouseTimer.isActive()) {
            m_delayedMouseTimer.start();
        }
    } else if (e->type() == QEvent::Show) {
        m_corona->wm()->setViewExtraFlags(this);
    }

    return QQuickView::event(e);
}

void ScreenEdgeGhostWindow::hideWithMask()
{
    //! old values: 0,0,1,1 were blocking the top-left corner of the window
    QRect maskGeometry{-2, 0, 1, 1};

    setMask(maskGeometry);
}

void ScreenEdgeGhostWindow::showWithMask()
{
    setMask(QRect());
}

}
}
