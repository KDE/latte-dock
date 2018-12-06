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
#include "../lattecorona.h"

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

ScreenEdgeGhostWindow::ScreenEdgeGhostWindow(Latte::View *view) :
    m_latteView(view)
{
    setColor(QColor(Qt::transparent));
    setDefaultAlphaBuffer(true);

    setFlags(Qt::FramelessWindowHint
             | Qt::WindowStaysOnTopHint
             | Qt::NoDropShadowWindowHint
             | Qt::WindowDoesNotAcceptFocus);

    m_fixGeometryTimer.setSingleShot(true);
    m_fixGeometryTimer.setInterval(500);
    connect(&m_fixGeometryTimer, &QTimer::timeout, this, &ScreenEdgeGhostWindow::fixGeometry);

    connect(this, &QQuickView::xChanged, this, &ScreenEdgeGhostWindow::startGeometryTimer);
    connect(this, &QQuickView::yChanged, this, &ScreenEdgeGhostWindow::startGeometryTimer);
    connect(this, &QQuickView::widthChanged, this, &ScreenEdgeGhostWindow::startGeometryTimer);
    connect(this, &QQuickView::heightChanged, this, &ScreenEdgeGhostWindow::startGeometryTimer);

    connect(m_latteView, &Latte::View::absGeometryChanged, this, &ScreenEdgeGhostWindow::updateGeometry);
    connect(m_latteView, &Latte::View::screenGeometryChanged, this, &ScreenEdgeGhostWindow::updateGeometry);
    connect(m_latteView, &Latte::View::locationChanged, this, &ScreenEdgeGhostWindow::updateGeometry);
    connect(m_latteView, &QQuickView::screenChanged, this, [this]() {
        setScreen(m_latteView->screen());
        updateGeometry();
    });


    if (!KWindowSystem::isPlatformWayland()) {
        connect(this, &QWindow::visibleChanged, this, [&]() {
            //! IMPORTANT!!! ::: This fixes a bug when closing an Activity all docks from all Activities are
            //!  disappearing! With this they reappear!!!
            if (m_latteView && m_latteView->managedLayout()) {
                if (!isVisible()) {
                    QTimer::singleShot(100, [this]() {
                        if (!m_inDelete && m_latteView && m_latteView->managedLayout() && !isVisible()) {
                            setVisible(true);
                        }
                    });

                    QTimer::singleShot(1500, [this]() {
                        if (!m_inDelete && m_latteView && m_latteView->managedLayout() && !isVisible()) {
                            setVisible(true);
                        }
                    });
                } else {
                    //! For some reason when the window is hidden in the edge under X11 afterwards
                    //! is losing its window flags
                    if (!m_inDelete) {
                        KWindowSystem::setType(winId(), NET::Dock);
                        KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
                        KWindowSystem::setOnAllDesktops(winId(), true);
                    }
                }
            }
        });
    }

    setupWaylandIntegration();

    setScreen(m_latteView->screen());
    setVisible(true);
    updateGeometry();
    hideWithMask();
}

ScreenEdgeGhostWindow::~ScreenEdgeGhostWindow()
{
    m_inDelete = true;
    m_latteView = nullptr;

    if (m_shellSurface) {
        delete m_shellSurface;
    }
}

int ScreenEdgeGhostWindow::location()
{
    return (int)m_latteView->location();
}

Latte::View *ScreenEdgeGhostWindow::parentDock()
{
    return m_latteView;
}

KWayland::Client::PlasmaShellSurface *ScreenEdgeGhostWindow::surface()
{
    return m_shellSurface;
}

void ScreenEdgeGhostWindow::updateGeometry()
{
    QRect newGeometry;
    int thickness{1};

    if (m_latteView->location() == Plasma::Types::BottomEdge) {
        newGeometry.setX(m_latteView->absGeometry().left());
        newGeometry.setY(m_latteView->screenGeometry().bottom() - thickness);
    } else if (m_latteView->location() == Plasma::Types::TopEdge) {
        newGeometry.setX(m_latteView->absGeometry().left());
        newGeometry.setY(m_latteView->screenGeometry().top());
    } else if (m_latteView->location() == Plasma::Types::LeftEdge) {
        newGeometry.setX(m_latteView->screenGeometry().left());
        newGeometry.setY(m_latteView->absGeometry().top());
    } else if (m_latteView->location() == Plasma::Types::RightEdge) {
        newGeometry.setX(m_latteView->screenGeometry().right() - thickness);
        newGeometry.setY(m_latteView->absGeometry().top());
    }

    if (m_latteView->formFactor() == Plasma::Types::Horizontal) {
        newGeometry.setWidth(qMin(m_latteView->absGeometry().width(), m_latteView->screenGeometry().width() - 1));
        newGeometry.setHeight(thickness + 1);
    } else {
        newGeometry.setWidth(thickness + 1);
        newGeometry.setHeight(qMin(m_latteView->absGeometry().height(), m_latteView->screenGeometry().height() - 1));
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

    if (Latte::Corona *c = qobject_cast<Latte::Corona *>(m_latteView->containment()->corona())) {
        using namespace KWayland::Client;

        PlasmaShell *interface = c->waylandCoronaInterface();

        if (!interface) {
            return;
        }

        Surface *s = Surface::fromWindow(this);

        if (!s) {
            return;
        }

        qDebug() << "wayland screen edge ghost window surface was created...";
        m_shellSurface = interface->createSurface(s, this);
        m_shellSurface->setSkipTaskbar(true);
        m_shellSurface->setPanelTakesFocus(false);
        m_shellSurface->setRole(PlasmaShellSurface::Role::Panel);
        m_shellSurface->setPanelBehavior(PlasmaShellSurface::PanelBehavior::AutoHide);
    }
}

bool ScreenEdgeGhostWindow::event(QEvent *e)
{
    if (e->type() == QEvent::Enter || e->type() == QEvent::DragEnter) {
        emit containsMouseChanged(true);
    } else if (e->type() == QEvent::Leave || e->type() == QEvent::DragLeave) {
        emit containsMouseChanged(false);
    }

    return QQuickView::event(e);
}

void ScreenEdgeGhostWindow::hideWithMask()
{
    QRect maskGeometry{0, 0, 1, 1};

    setMask(maskGeometry);
}

void ScreenEdgeGhostWindow::showWithMask()
{
    setMask(QRect());
}

}
