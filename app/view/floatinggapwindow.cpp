/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
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

#include "floatinggapwindow.h"

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

FloatingGapWindow::FloatingGapWindow(Latte::View *view) :
    m_latteView(view)
{
    m_corona = qobject_cast<Latte::Corona *>(view->corona());

    m_debugMode = (qApp->arguments().contains("-d") && qApp->arguments().contains("--kwinedges"));

    if (m_debugMode) {
        m_showColor = QColor("green");
        m_hideColor = QColor("red");
    } else {
        m_showColor = QColor(Qt::transparent);
        m_hideColor = QColor(Qt::transparent);

        m_showColor.setAlpha(0);
        m_hideColor.setAlpha(1);
    }

    setColor(m_showColor);
    setDefaultAlphaBuffer(true);

    setFlags(Qt::FramelessWindowHint
             | Qt::WindowStaysOnTopHint
             | Qt::NoDropShadowWindowHint
             | Qt::WindowDoesNotAcceptFocus);

    m_fixGeometryTimer.setSingleShot(true);
    m_fixGeometryTimer.setInterval(500);
    connect(&m_fixGeometryTimer, &QTimer::timeout, this, &FloatingGapWindow::fixGeometry);

    //! this timer is used in order to identify if mouse is still present in sensitive floating
    //! areas and in such case to prevent a real-floating view to hide itself
    m_asyncMouseTimer.setSingleShot(true);
    m_asyncMouseTimer.setInterval(200);
    connect(&m_asyncMouseTimer, &QTimer::timeout, this, [this]() {
        if (m_inAsyncContainsMouse && !m_containsMouse) {
            emit asyncContainsMouseChanged(false);
            hideWithMask();
            m_inAsyncContainsMouse = false;
        }
    });

    connect(this, &QQuickView::xChanged, this, &FloatingGapWindow::startGeometryTimer);
    connect(this, &QQuickView::yChanged, this, &FloatingGapWindow::startGeometryTimer);
    connect(this, &QQuickView::widthChanged, this, &FloatingGapWindow::startGeometryTimer);
    connect(this, &QQuickView::heightChanged, this, &FloatingGapWindow::startGeometryTimer);

    connect(m_latteView, &Latte::View::absoluteGeometryChanged, this, &FloatingGapWindow::updateGeometry);
    connect(m_latteView, &Latte::View::screenGeometryChanged, this, &FloatingGapWindow::updateGeometry);
    connect(m_latteView, &Latte::View::locationChanged, this, &FloatingGapWindow::updateGeometry);
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
                //qDebug() << "Floating Gap:: Enforce reshow from timer 1...";
            } else {
                //qDebug() << "Floating Gap:: No needed reshow from timer 1...";
            }
        });

        connectionsHack << connect(&m_visibleHackTimer2, &QTimer::timeout, this, [&]() {
            if (!m_inDelete && m_latteView && m_latteView->layout() && !isVisible()) {
                show();
                emit forcedShown();
                //qDebug() << "Floating Gap:: Enforce reshow from timer 2...";
            } else {
                //qDebug() << "Floating Gap:: No needed reshow from timer 2...";
            }
        });

        connectionsHack << connect(this, &FloatingGapWindow::forcedShown, this, [&]() {
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

FloatingGapWindow::~FloatingGapWindow()
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

int FloatingGapWindow::location()
{
    return (int)m_latteView->location();
}

int FloatingGapWindow::thickness() const
{
    return m_thickness;
}

Latte::View *FloatingGapWindow::parentView()
{
    return m_latteView;
}

KWayland::Client::PlasmaShellSurface *FloatingGapWindow::surface()
{
    return m_shellSurface;
}

void FloatingGapWindow::updateGeometry()
{
    if (m_latteView->positioner()->slideOffset() != 0) {
        return;
    }

    QRect newGeometry;

    m_thickness = m_latteView->screenEdgeMargin();

    int length = m_latteView->formFactor() == Plasma::Types::Horizontal ? m_latteView->absoluteGeometry().width() : m_latteView->absoluteGeometry().height();

    if (m_latteView->location() == Plasma::Types::BottomEdge) {
        int xF = qMax(m_latteView->screenGeometry().left(), m_latteView->absoluteGeometry().left());
        newGeometry.setX(xF);
        newGeometry.setY(m_latteView->screenGeometry().bottom() - m_thickness);
    } else if (m_latteView->location() == Plasma::Types::TopEdge) {
        int xF = qMax(m_latteView->screenGeometry().left(), m_latteView->absoluteGeometry().left());
        newGeometry.setX(xF);
        newGeometry.setY(m_latteView->screenGeometry().top());
    } else if (m_latteView->location() == Plasma::Types::LeftEdge) {
        int yF = qMax(m_latteView->screenGeometry().top(), m_latteView->absoluteGeometry().top());
        newGeometry.setX(m_latteView->screenGeometry().left());
        newGeometry.setY(yF);
    } else if (m_latteView->location() == Plasma::Types::RightEdge) {
        int yF = qMax(m_latteView->screenGeometry().top(), m_latteView->absoluteGeometry().top());
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

void FloatingGapWindow::fixGeometry()
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

void FloatingGapWindow::startGeometryTimer()
{
    m_fixGeometryTimer.start();
}

void FloatingGapWindow::setupWaylandIntegration()
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

bool FloatingGapWindow::containsMouse() const
{
    return m_containsMouse;
}

void FloatingGapWindow::setContainsMouse(bool contains)
{
    if (m_containsMouse == contains) {
        return;
    }

    m_containsMouse = contains;
}

bool FloatingGapWindow::event(QEvent *e)
{
    if (e->type() == QEvent::DragEnter || e->type() == QEvent::DragMove) {
        setContainsMouse(true);
        emit dragEntered();
    } else if (e->type() == QEvent::Enter) {
        setContainsMouse(true);
        triggerAsyncContainsMouseSignals();
    } else if (e->type() == QEvent::Leave || e->type() == QEvent::DragLeave) {
        setContainsMouse(false);
        if (m_inAsyncContainsMouse) {
            m_asyncMouseTimer.stop();
            m_inAsyncContainsMouse = false;
            emit asyncContainsMouseChanged(true);
        }
    } else if (e->type() == QEvent::Show) {
        m_corona->wm()->setViewExtraFlags(this);
    }

    return QQuickView::event(e);
}

void FloatingGapWindow::callAsyncContainsMouse()
{
    m_inAsyncContainsMouse = true;
    m_asyncMouseTimer.start();
    showWithMask();
}

void FloatingGapWindow::triggerAsyncContainsMouseSignals()
{
    if (!m_inAsyncContainsMouse) {
        return;
    }

    //! this function is called QEvent::Enter
    m_asyncMouseTimer.stop();
    hideWithMask();
}

void FloatingGapWindow::hideWithMask()
{
    if (m_debugMode) {
        qDebug() << " Floating Gap Window :: MASK HIDE...";
    }

    //! old values: 0,0,1,1 were blocking the top-left corner of the window
    QRect maskGeometry{-2, 0, 1, 1};
    setMask(maskGeometry);

    //! repaint in order to update mask immediately
    setColor(m_hideColor);
}

void FloatingGapWindow::showWithMask()
{
    if (m_debugMode) {
        qDebug() << " Floating Gap Window :: MAKS SHOW...";
    }

    setMask(QRegion());

    //! repaint in order to update mask immediately
    setColor(m_showColor);
}

}
}
