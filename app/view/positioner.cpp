/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "positioner.h"

// local
#include "effects.h"
#include "view.h"
#include "../dockcorona.h"
#include "../screenpool.h"
#include "../settings/universalsettings.h"
#include "../../liblattedock/dock.h"

// Qt
#include <QDebug>

// KDE
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem>

namespace Latte {
namespace ViewPart {

Positioner::Positioner(Latte::View *parent)
    : QObject(parent),
      m_view(parent)
{
    m_screenSyncTimer.setSingleShot(true);
    m_screenSyncTimer.setInterval(2000);
    connect(&m_screenSyncTimer, &QTimer::timeout, this, &Positioner::reconsiderScreen);

    //! under X11 it was identified that windows many times especially under screen changes
    //! don't end up at the correct position and size. This timer will enforce repositionings
    //! and resizes every 500ms if the window hasn't end up to correct values and until this
    //! is achieved
    m_validateGeometryTimer.setSingleShot(true);
    m_validateGeometryTimer.setInterval(500);
    connect(&m_validateGeometryTimer, &QTimer::timeout, this, &Positioner::syncGeometry);

    auto *dockCorona = qobject_cast<DockCorona *>(m_view->corona());

    if (dockCorona) {
        m_screenSyncTimer.setInterval(qMax(dockCorona->universalSettings()->screenTrackerInterval() - 500, 1000));
        connect(dockCorona->universalSettings(), &UniversalSettings::screenTrackerIntervalChanged, this, [this, dockCorona]() {
            m_screenSyncTimer.setInterval(qMax(dockCorona->universalSettings()->screenTrackerInterval() - 500, 1000));
        });

        connect(dockCorona, &DockCorona::dockLocationChanged, this, [&]() {
            //! check if an edge has been freed for a primary dock
            //! from another screen
            if (m_view->onPrimary()) {
                m_screenSyncTimer.start();
            }
        });
    }

    init();
}

Positioner::~Positioner()
{
    m_inDelete = true;

    m_screenSyncTimer.stop();
    m_validateGeometryTimer.stop();
}

void Positioner::init()
{
    //! connections
    connect(this, &Positioner::screenGeometryChanged, this, &Positioner::syncGeometry);

    connect(m_view, &QQuickWindow::xChanged, this, &Positioner::validateDockGeometry);
    connect(m_view, &QQuickWindow::yChanged, this, &Positioner::validateDockGeometry);
    connect(m_view, &QQuickWindow::widthChanged, this, &Positioner::validateDockGeometry);
    connect(m_view, &QQuickWindow::heightChanged, this, &Positioner::validateDockGeometry);
    connect(m_view, &QQuickWindow::screenChanged, this, &Positioner::currentScreenChanged);
    connect(m_view, &QQuickWindow::screenChanged, this, &Positioner::screenChanged);

    connect(m_view, &Latte::View::absGeometryChanged, this, &Positioner::syncGeometry);
    connect(m_view, &Latte::View::behaveAsPlasmaPanelChanged, this, &Positioner::syncGeometry);
    connect(m_view, &Latte::View::maxThicknessChanged, this, &Positioner::syncGeometry);
    connect(m_view, &Latte::View::maxLengthChanged, this, &Positioner::syncGeometry);
    connect(m_view, &Latte::View::offsetChanged, this, &Positioner::syncGeometry);

    connect(m_view, &Latte::View::locationChanged, this, [&]() {
        updateFormFactor();
        syncGeometry();
    });

    connect(m_view, &Latte::View::normalThicknessChanged, this, [&]() {
        if (m_view->behaveAsPlasmaPanel()) {
            syncGeometry();
        }
    });

    connect(m_view->effects(), &Latte::ViewPart::Effects::drawShadowsChanged, this, [&]() {
        if (!m_view->behaveAsPlasmaPanel()) {
            syncGeometry();
        }
    });

    connect(m_view->effects(), &Latte::ViewPart::Effects::innerShadowChanged, this, [&]() {
        if (m_view->behaveAsPlasmaPanel()) {
            syncGeometry();
        }
    });

    connect(qGuiApp, &QGuiApplication::screenAdded, this, &Positioner::screenChanged);
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &Positioner::screenChanged);

    initSignalingForLocationChangeSliding();
}

int Positioner::currentScreenId() const
{
    auto *dockCorona = qobject_cast<DockCorona *>(m_view->corona());

    if (dockCorona) {
        return dockCorona->screenPool()->id(m_screenToFollowId);
    }

    return -1;
}

QString Positioner::currentScreenName() const
{
    return m_screenToFollowId;
}

bool Positioner::setCurrentScreen(const QString id)
{
    QScreen *nextScreen{qGuiApp->primaryScreen()};

    if (id != "primary") {
        foreach (auto scr, qGuiApp->screens()) {
            if (scr && scr->name() == id) {
                nextScreen = scr;
                break;
            }
        }
    }

    if (m_screenToFollow == nextScreen) {
        return true;
    }

    if (nextScreen) {
        if (m_view->managedLayout()) {
            auto freeEdges = m_view->managedLayout()->freeEdges(nextScreen);

            if (!freeEdges.contains(m_view->location())) {
                return false;
            } else {
                m_goToScreen = nextScreen;

                //! asynchronous call in order to not crash from configwindow
                //! deletion from sliding out animation
                QTimer::singleShot(100, [this]() {
                    emit hideDockDuringScreenChangeStarted();
                });
            }
        }
    }

    return true;
}

//! this function updates the dock's associated screen.
//! updateScreenId = true, update also the m_screenToFollowId
//! updateScreenId = false, do not update the m_screenToFollowId
//! that way an explicit dock can be shown in another screen when
//! there isnt a tasks dock running in the system and for that
//! dock its first origin screen is stored and that way when
//! that screen is reconnected the dock will return to its original
//! place
void Positioner::setScreenToFollow(QScreen *scr, bool updateScreenId)
{
    if (!scr || (scr && (m_screenToFollow == scr) && (m_view->screen() == scr))) {
        return;
    }

    qDebug() << "setScreenToFollow() called for screen:" << scr->name() << " update:" << updateScreenId;

    m_screenToFollow = scr;

    if (updateScreenId) {
        m_screenToFollowId = scr->name();
    }

    qDebug() << "adapting to screen...";
    m_view->setScreen(scr);

    if (m_view->containment()) {
        m_view->containment()->reactToScreenChange();
    }

    connect(scr, &QScreen::geometryChanged, this, &Positioner::screenGeometryChanged);
    syncGeometry();
    m_view->updateAbsDockGeometry(true);
    qDebug() << "setScreenToFollow() ended...";

    emit screenGeometryChanged();
    emit currentScreenChanged();
}

//! the main function which decides if this dock is at the
//! correct screen
void Positioner::reconsiderScreen()
{
    if (m_inDelete) {
        return;
    }

    qDebug() << "reconsiderScreen() called...";
    qDebug() << "  Delayer  ";

    foreach (auto scr, qGuiApp->screens()) {
        qDebug() << "      D, found screen: " << scr->name();
    }

    bool screenExists{false};

    //!check if the associated screen is running
    foreach (auto scr, qGuiApp->screens()) {
        if (m_screenToFollowId == scr->name()
            || (m_view->onPrimary() && scr == qGuiApp->primaryScreen())) {
            screenExists = true;
        }
    }

    qDebug() << "dock screen exists  ::: " << screenExists;

    //! 1.a primary dock must be always on the primary screen
    if (m_view->onPrimary() && (m_screenToFollowId != qGuiApp->primaryScreen()->name()
                                || m_screenToFollow != qGuiApp->primaryScreen()
                                || m_view->screen() != qGuiApp->primaryScreen())) {
        using Plasma::Types;
        QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                                     Types::TopEdge, Types::RightEdge};

        edges = m_view->managedLayout() ? m_view->managedLayout()->availableEdgesForView(qGuiApp->primaryScreen(), m_view) : edges;

        //change to primary screen only if the specific edge is free
        qDebug() << "updating the primary screen for dock...";
        qDebug() << "available primary screen edges:" << edges;
        qDebug() << "dock location:" << m_view->location();

        if (edges.contains(m_view->location())) {
            //! case 1
            qDebug() << "reached case 1: of updating dock primary screen...";
            setScreenToFollow(qGuiApp->primaryScreen());
        }
    } else if (!m_view->onPrimary()) {
        //! 2.an explicit dock must be always on the correct associated screen
        //! there are cases that window manager misplaces the dock, this function
        //! ensures that this dock will return at its correct screen
        foreach (auto scr, qGuiApp->screens()) {
            if (scr && scr->name() == m_screenToFollowId) {
                qDebug() << "reached case 2: updating the explicit screen for dock...";
                setScreenToFollow(scr);
                break;
            }
        }
    }

    syncGeometry();
    qDebug() << "reconsiderScreen() ended...";
}

void Positioner::screenChanged(QScreen *scr)
{
    m_screenSyncTimer.start();

    //! this is needed in order to update the struts on screen change
    //! and even though the geometry has been set correctly the offsets
    //! of the screen must be updated to the new ones
    if (m_view->visibility() && m_view->visibility()->mode() == Latte::Dock::AlwaysVisible) {
        m_view->updateAbsDockGeometry(true);
    }
}

void Positioner::syncGeometry()
{
    if (!(m_view->screen() && m_view->containment()) || m_inDelete) {
        return;
    }

    bool found{false};

    qDebug() << "syncGeometry() called...";

    //! before updating the positioning and geometry of the dock
    //! we make sure that the dock is at the correct screen
    if (m_view->screen() != m_screenToFollow) {
        qDebug() << "Sync Geometry screens inconsistent!!!! ";

        if (m_screenToFollow) {
            qDebug() << "Sync Geometry screens inconsistent for m_screenToFollow:" << m_screenToFollow->name() << " dock screen:" << m_view->screen()->name();
        }

        if (!m_screenSyncTimer.isActive()) {
            m_screenSyncTimer.start();
        }
    } else {
        found = true;
    }

    //! if the dock isnt at the correct screen the calculations
    //! are not executed
    if (found) {
        //! compute the free screen rectangle for vertical panels only once
        //! this way the costly QRegion computations are calculated only once
        //! instead of two times (both inside the resizeWindow and the updatePosition)
        QRegion freeRegion;;
        QRect maximumRect;
        QRect availableScreenRect{m_view->screen()->geometry()};

        if (m_view->formFactor() == Plasma::Types::Vertical) {
            QString layoutName = m_view->managedLayout() ? m_view->managedLayout()->name() : QString();
            auto dockCorona = qobject_cast<DockCorona *>(m_view->corona());
            int fixedScreen = m_view->onPrimary() ? dockCorona->screenPool()->primaryScreenId() : m_view->containment()->screen();

            freeRegion = dockCorona->availableScreenRegionWithCriteria(fixedScreen, layoutName);
            maximumRect = maximumNormalGeometry();
            QRegion availableRegion = freeRegion.intersected(maximumRect);
            availableScreenRect = freeRegion.intersected(maximumRect).boundingRect();
            float area = 0;

            //! it is used to choose which or the availableRegion rectangles will
            //! be the one representing dock geometry
            for (int i = 0; i < availableRegion.rectCount(); ++i) {
                QRect rect = availableRegion.rects().at(i);
                //! the area of each rectangle in calculated in squares of 50x50
                //! this is a way to avoid enourmous numbers for area value
                float tempArea = (float)(rect.width() * rect.height()) / 2500;

                if (tempArea > area) {
                    availableScreenRect = rect;
                    area = tempArea;
                }
            }

            if (availableRegion.rectCount() > 1 && m_view->behaveAsPlasmaPanel()) {
                m_view->effects()->setForceDrawCenteredBorders(true);
            } else {
                m_view->effects()->setForceDrawCenteredBorders(false);
            }
        } else {
            m_view->effects()->setForceDrawCenteredBorders(false);
        }

        m_view->effects()->updateEnabledBorders();
        resizeWindow(availableScreenRect);
        updatePosition(availableScreenRect);

        qDebug() << "syncGeometry() calculations for screen: " << m_view->screen()->name() << " _ " << m_view->screen()->geometry();
    }

    qDebug() << "syncGeometry() ended...";

    // qDebug() << "dock geometry:" << qRectToStr(geometry());
}

void Positioner::validateDockGeometry()
{
    if (m_view->geometry() != m_validGeometry) {
        m_validateGeometryTimer.start();
    }
}

//! this is used mainly from vertical panels in order to
//! to get the maximum geometry that can be used from the dock
//! based on their alignment type and the location dock
QRect Positioner::maximumNormalGeometry()
{
    int xPos = 0;
    int yPos = 0;
    int maxHeight = m_view->maxLength() * m_view->screen()->geometry().height();
    int maxWidth = m_view->normalThickness();
    QRect maxGeometry;
    maxGeometry.setRect(0, 0, maxWidth, maxHeight);

    switch (m_view->location()) {
        case Plasma::Types::LeftEdge:
            xPos = m_view->screen()->geometry().x();

            switch (m_view->alignment()) {
                case Latte::Dock::Top:
                    yPos = m_view->screen()->geometry().y();
                    break;

                case Latte::Dock::Center:
                case Latte::Dock::Justify:
                    yPos = qMax(m_view->screen()->geometry().center().y() - maxHeight / 2, m_view->screen()->geometry().y());
                    break;

                case Latte::Dock::Bottom:
                    yPos = m_view->screen()->geometry().bottom() - maxHeight + 1;
                    break;
            }

            maxGeometry.setRect(xPos, yPos, maxWidth, maxHeight);
            break;

        case Plasma::Types::RightEdge:
            xPos = m_view->screen()->geometry().right() - maxWidth + 1;

            switch (m_view->alignment()) {
                case Latte::Dock::Top:
                    yPos = m_view->screen()->geometry().y();
                    break;

                case Latte::Dock::Center:
                case Latte::Dock::Justify:
                    yPos = qMax(m_view->screen()->geometry().center().y() - maxHeight / 2, m_view->screen()->geometry().y());
                    break;

                case Latte::Dock::Bottom:
                    yPos = m_view->screen()->geometry().bottom() - maxHeight + 1;
                    break;
            }

            maxGeometry.setRect(xPos, yPos, maxWidth, maxHeight);
            break;

        default:
            //! bypass clang warnings
            break;
    }

    //! this is needed in order to preserve that the top dock will be above
    //! the others in case flag bypasswindowmanagerhint hasn't be set,
    //! such a case is the AlwaysVisible mode
    if (m_view->location() == Plasma::Types::TopEdge) {
        KWindowSystem::setState(m_view->winId(), NET::KeepAbove);
    } else {
        KWindowSystem::clearState(m_view->winId(), NET::KeepAbove);
    }

    return maxGeometry;
}

void Positioner::updatePosition(QRect availableScreenRect)
{
    QRect screenGeometry{availableScreenRect};
    QPoint position;
    position = {0, 0};

    const auto length = [&](int length) -> int {
        float offs = static_cast<float>(m_view->offset());
        return static_cast<int>(length * ((1 - m_view->maxLength()) / 2) + length * (offs / 100));
    };
    int cleanThickness = m_view->normalThickness() - m_view->effects()->innerShadow();

    switch (m_view->location()) {
        case Plasma::Types::TopEdge:
            if (m_view->behaveAsPlasmaPanel()) {
                position = {screenGeometry.x() + length(screenGeometry.width()), screenGeometry.y()};
            } else {
                position = {screenGeometry.x(), screenGeometry.y()};
            }

            break;

        case Plasma::Types::BottomEdge:
            if (m_view->behaveAsPlasmaPanel()) {
                position = {screenGeometry.x() + length(screenGeometry.width()),
                            screenGeometry.y() + screenGeometry.height() - cleanThickness
                           };
            } else {
                position = {screenGeometry.x(), screenGeometry.y() + screenGeometry.height() - m_view->height()};
            }

            break;

        case Plasma::Types::RightEdge:
            if (m_view->behaveAsPlasmaPanel() && !m_view->mask().isNull()) {
                position = {availableScreenRect.right() - cleanThickness + 1,
                            availableScreenRect.y() + length(availableScreenRect.height())
                           };
            } else {
                position = {availableScreenRect.right() - m_view->width() + 1, availableScreenRect.y()};
            }

            break;

        case Plasma::Types::LeftEdge:
            if (m_view->behaveAsPlasmaPanel() && !m_view->mask().isNull()) {
                position = {availableScreenRect.x(), availableScreenRect.y() + length(availableScreenRect.height())};
            } else {
                position = {availableScreenRect.x(), availableScreenRect.y()};
            }

            break;

        default:
            qWarning() << "wrong location, couldn't update the panel position"
                       << m_view->location();
    }

    m_validGeometry.setTopLeft(position);

    m_view->setPosition(position);

    if (m_view->surface()) {
        m_view->surface()->setPosition(position);
    }
}

void Positioner::resizeWindow(QRect availableScreenRect)
{
    QSize screenSize = m_view->screen()->size();
    QSize size = (m_view->formFactor() == Plasma::Types::Vertical) ? QSize(m_view->maxThickness(), availableScreenRect.height()) : QSize(screenSize.width(), m_view->maxThickness());

    if (m_view->formFactor() == Plasma::Types::Vertical) {
        //qDebug() << "MAXIMUM RECT :: " << maximumRect << " - AVAILABLE RECT :: " << availableRect;
        if (m_view->behaveAsPlasmaPanel()) {
            size.setWidth(m_view->normalThickness());
            size.setHeight(static_cast<int>(m_view->maxLength() * availableScreenRect.height()));
        }
    } else {
        if (m_view->behaveAsPlasmaPanel()) {
            size.setWidth(static_cast<int>(m_view->maxLength() * screenSize.width()));
            size.setHeight(m_view->normalThickness());
        }
    }

    m_validGeometry.setSize(size);

    m_view->setMinimumSize(size);
    m_view->setMaximumSize(size);
    m_view->resize(size);

    if (m_view->formFactor() == Plasma::Types::Horizontal && m_view->corona()) {
        emit m_view->corona()->availableScreenRectChanged();
    }
}

void Positioner::updateFormFactor()
{
    if (!m_view->containment())
        return;

    switch (m_view->location()) {
        case Plasma::Types::TopEdge:
        case Plasma::Types::BottomEdge:
            m_view->containment()->setFormFactor(Plasma::Types::Horizontal);
            break;

        case Plasma::Types::LeftEdge:
        case Plasma::Types::RightEdge:
            m_view->containment()->setFormFactor(Plasma::Types::Vertical);
            break;

        default:
            qWarning() << "wrong location, couldn't update the panel position" << m_view->location();
    }
}

void Positioner::initSignalingForLocationChangeSliding()
{
    //! signals to handle the sliding-in/out during location changes
    connect(this, &Positioner::hideDockDuringLocationChangeStarted, this, &Positioner::onHideWindowsForSlidingOut);

    connect(m_view, &View::locationChanged, this, [&]() {
        if (m_goToLocation != Plasma::Types::Floating) {
            m_goToLocation = Plasma::Types::Floating;
            QTimer::singleShot(100, [this]() {
                m_view->effects()->setAnimationsBlocked(false);
                emit showDockAfterLocationChangeFinished();
                m_view->showSettingsWindow();

                if (m_view->managedLayout()) {
                    m_view->managedLayout()->syncDockViewsToScreens();
                }
            });
        }
    });

    //! signals to handle the sliding-in/out during screen changes
    connect(this, &Positioner::hideDockDuringScreenChangeStarted, this, &Positioner::onHideWindowsForSlidingOut);

    connect(this, &Positioner::currentScreenChanged, this, [&]() {
        if (m_goToScreen) {
            m_goToScreen = nullptr;
            QTimer::singleShot(100, [this]() {
                m_view->effects()->setAnimationsBlocked(false);
                emit showDockAfterScreenChangeFinished();
                m_view->showSettingsWindow();

                if (m_view->managedLayout()) {
                    m_view->managedLayout()->syncDockViewsToScreens();
                }
            });
        }
    });

    //! signals to handle the sliding-in/out during moving to another layout
    connect(this, &Positioner::hideDockDuringMovingToLayoutStarted, this, &Positioner::onHideWindowsForSlidingOut);

    connect(m_view, &View::managedLayoutChanged, this, [&]() {
        if (!m_moveToLayout.isEmpty() && m_view->managedLayout()) {
            m_moveToLayout = "";
            QTimer::singleShot(100, [this]() {
                m_view->effects()->setAnimationsBlocked(false);
                emit showDockAfterMovingToLayoutFinished();
                m_view->showSettingsWindow();
            });
        }
    });

    //!    ----  both cases   ----  !//
    //! this is used for both location and screen change cases, this signal
    //! is send when the sliding-out animation has finished
    connect(this, &Positioner::hideDockDuringLocationChangeFinished, this, [&]() {
        m_view->effects()->setAnimationsBlocked(true);

        if (m_goToLocation != Plasma::Types::Floating) {
            m_view->setLocation(m_goToLocation);
        } else if (m_goToScreen) {
            setScreenToFollow(m_goToScreen);
        } else if (!m_moveToLayout.isEmpty()) {
            m_view->moveToLayout(m_moveToLayout);
        }
    });
}

bool Positioner::inLocationChangeAnimation()
{
    return ((m_goToLocation != Plasma::Types::Floating) || (m_moveToLayout != "") || m_goToScreen);
}

void Positioner::hideDockDuringLocationChange(int goToLocation)
{
    m_goToLocation = static_cast<Plasma::Types::Location>(goToLocation);
    emit hideDockDuringLocationChangeStarted();
}

void Positioner::hideDockDuringMovingToLayout(QString layoutName)
{
    m_moveToLayout = layoutName;
    emit hideDockDuringMovingToLayoutStarted();
}

}
}
