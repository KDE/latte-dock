/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "positioner.h"

// local
#include <coretypes.h>
#include "effects.h"
#include "originalview.h"
#include "view.h"
#include "visibilitymanager.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../data/screendata.h"
#include "../layout/centrallayout.h"
#include "../layouts/manager.h"
#include "../settings/universalsettings.h"
#include "../wm/abstractwindowinterface.h"

// Qt
#include <QDebug>

// KDE
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem>

#define RELOCATIONSHOWINGEVENT "viewInRelocationShowing"

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

    //! syncGeometry() function is costly, so now we make sure that is not executed too often
    m_syncGeometryTimer.setSingleShot(true);
    m_syncGeometryTimer.setInterval(150);
    connect(&m_syncGeometryTimer, &QTimer::timeout, this, &Positioner::immediateSyncGeometry);

    m_corona = qobject_cast<Latte::Corona *>(m_view->corona());

    if (m_corona) {
        if (KWindowSystem::isPlatformX11()) {
            m_trackedWindowId = m_view->winId();
            m_corona->wm()->registerIgnoredWindow(m_trackedWindowId);

            connect(m_view, &Latte::View::forcedShown, this, [&]() {
                m_corona->wm()->unregisterIgnoredWindow(m_trackedWindowId);
                m_trackedWindowId = m_view->winId();
                m_corona->wm()->registerIgnoredWindow(m_trackedWindowId);
            });
        } else {
            connect(m_view, &QWindow::windowTitleChanged, this, &Positioner::updateWaylandId);
            connect(m_corona->wm(), &WindowSystem::AbstractWindowInterface::latteWindowAdded, this, &Positioner::updateWaylandId);
        }

        connect(m_corona->layoutsManager(), &Layouts::Manager::currentLayoutIsSwitching, this, &Positioner::onCurrentLayoutIsSwitching);
        /////

        m_screenSyncTimer.setInterval(qMax(m_corona->universalSettings()->screenTrackerInterval() - 500, 1000));
        connect(m_corona->universalSettings(), &UniversalSettings::screenTrackerIntervalChanged, this, [&]() {
            m_screenSyncTimer.setInterval(qMax(m_corona->universalSettings()->screenTrackerInterval() - 500, 1000));
        });

        connect(m_corona, &Latte::Corona::viewLocationChanged, this, [&]() {
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
    slideOutDuringExit();
    m_corona->wm()->unregisterIgnoredWindow(m_trackedWindowId);

    m_screenSyncTimer.stop();
    m_validateGeometryTimer.stop();
}

void Positioner::init()
{
    //! connections
    connect(this, &Positioner::screenGeometryChanged, this, &Positioner::syncGeometry);

    connect(this, &Positioner::hidingForRelocationStarted, this, &Positioner::updateInRelocationAnimation);
    connect(this, &Positioner::showingAfterRelocationFinished, this, &Positioner::updateInRelocationAnimation);
    connect(this, &Positioner::showingAfterRelocationFinished, this, &Positioner::syncLatteViews);
    connect(this, &Positioner::startupFinished, this, &Positioner::onStartupFinished);

    connect(m_view, &Latte::View::onPrimaryChanged, this, &Positioner::syncLatteViews);

    connect(this, &Positioner::inSlideAnimationChanged, this, [&]() {
        if (!inSlideAnimation()) {
            syncGeometry();
        }
    });

    connect(this, &Positioner::isStickedOnTopEdgeChanged, this, [&]() {
        if (m_view->formFactor() == Plasma::Types::Vertical) {
            syncGeometry();
        }
    });

    connect(this, &Positioner::isStickedOnBottomEdgeChanged, this, [&]() {
        if (m_view->formFactor() == Plasma::Types::Vertical) {
            syncGeometry();
        }
    });

    connect(m_corona->activitiesConsumer(), &KActivities::Consumer::currentActivityChanged, this, [&]() {
        if (m_view->formFactor() == Plasma::Types::Vertical && m_view->layout() && m_view->layout()->isCurrent()) {
            syncGeometry();
        }
    });

    connect(this, &Positioner::slideOffsetChanged, this, [&]() {
        updatePosition(m_lastAvailableScreenRect);
    });

    connect(m_view, &QQuickWindow::xChanged, this, &Positioner::validateDockGeometry);
    connect(m_view, &QQuickWindow::yChanged, this, &Positioner::validateDockGeometry);
    connect(m_view, &QQuickWindow::widthChanged, this, &Positioner::validateDockGeometry);
    connect(m_view, &QQuickWindow::heightChanged, this, &Positioner::validateDockGeometry);
    connect(m_view, &QQuickWindow::screenChanged, this, &Positioner::currentScreenChanged);
    connect(m_view, &QQuickWindow::screenChanged, this, &Positioner::onScreenChanged);

    connect(m_view, &Latte::View::behaveAsPlasmaPanelChanged, this, &Positioner::syncGeometry);
    connect(m_view, &Latte::View::maxThicknessChanged, this, &Positioner::syncGeometry);

    connect(m_view, &Latte::View::behaveAsPlasmaPanelChanged,  this, [&]() {
        if (!m_view->behaveAsPlasmaPanel() && m_slideOffset != 0) {
            m_slideOffset = 0;
            syncGeometry();
        }
    });

    connect(m_view, &Latte::View::offsetChanged, this, [&]() {
        updatePosition(m_lastAvailableScreenRect);
    });

    connect(m_view, &Latte::View::locationChanged, this, [&]() {
        updateFormFactor();
        syncGeometry();
    });

    connect(m_view, &Latte::View::editThicknessChanged, this, [&]() {
        updateCanvasGeometry(m_lastAvailableScreenRect);
    });

    connect(m_view, &Latte::View::maxLengthChanged, this, [&]() {
        if (m_view->behaveAsPlasmaPanel()) {
            syncGeometry();
        }
    });

    connect(m_view, &Latte::View::normalThicknessChanged, this, [&]() {
        if (m_view->behaveAsPlasmaPanel()) {
            syncGeometry();
        }
    });

    connect(m_view, &Latte::View::screenEdgeMarginEnabledChanged, this, [&]() {
        syncGeometry();
    });

    connect(m_view, &Latte::View::screenEdgeMarginChanged, this, [&]() {
        syncGeometry();
    });

    connect(m_view, &View::layoutChanged, this, [&]() {
        if (m_nextLayoutName.isEmpty() && m_view->layout() && m_view->formFactor() == Plasma::Types::Vertical) {
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

    connect(qGuiApp, &QGuiApplication::screenAdded, this, &Positioner::onScreenChanged);
    connect(m_corona->screenPool(), &ScreenPool::primaryScreenChanged, this, &Positioner::onScreenChanged);

    connect(m_view, &Latte::View::visibilityChanged, this, &Positioner::initDelayedSignals);

    initSignalingForLocationChangeSliding();
}

void Positioner::initDelayedSignals()
{
    connect(m_view->visibility(), &ViewPart::VisibilityManager::isHiddenChanged, this, [&]() {
        if (m_view->behaveAsPlasmaPanel() && !m_view->visibility()->isHidden() && qAbs(m_slideOffset)>0) {
            //! ignore any checks to make sure the panel geometry is up-to-date
            immediateSyncGeometry();
        }
    });
}

void Positioner::updateWaylandId()
{
    QString validTitle = m_view->validTitle();
    if (validTitle.isEmpty()) {
        return;
    }

    Latte::WindowSystem::WindowId newId = m_corona->wm()->winIdFor("latte-dock", validTitle);

    if (m_trackedWindowId != newId) {
        if (!m_trackedWindowId.isNull()) {
            m_corona->wm()->unregisterIgnoredWindow(m_trackedWindowId);
        }

        m_trackedWindowId = newId;
        m_corona->wm()->registerIgnoredWindow(m_trackedWindowId);

        emit winIdChanged();
    }
}

bool Positioner::inRelocationShowing() const
{
    return m_inRelocationShowing;
}

void Positioner::setInRelocationShowing(bool active)
{
    if (m_inRelocationShowing == active) {
        return;
    }

    m_inRelocationShowing = active;

    if (m_inRelocationShowing) {
        m_view->visibility()->addBlockHidingEvent(RELOCATIONSHOWINGEVENT);
    } else {
        m_view->visibility()->removeBlockHidingEvent(RELOCATIONSHOWINGEVENT);
    }
}

bool Positioner::isOffScreen() const
{
    return (m_view->absoluteGeometry().x()<-500 || m_view->absoluteGeometry().y()<-500);
}

int Positioner::currentScreenId() const
{
    auto *latteCorona = qobject_cast<Latte::Corona *>(m_view->corona());

    if (latteCorona) {
        return latteCorona->screenPool()->id(m_screenNameToFollow);
    }

    return -1;
}

Latte::WindowSystem::WindowId Positioner::trackedWindowId()
{
    if (KWindowSystem::isPlatformWayland() && m_trackedWindowId.toInt() <= 0) {
        updateWaylandId();
    }

    return m_trackedWindowId;
}

QString Positioner::currentScreenName() const
{
    return m_screenNameToFollow;
}

WindowSystem::AbstractWindowInterface::Slide Positioner::slideLocation(Plasma::Types::Location location)
{
    auto slideedge = WindowSystem::AbstractWindowInterface::Slide::None;

    if (location == Plasma::Types::Floating && m_view->containment()) {
        location = m_view->containment()->location();
    }

    switch (location) {
    case Plasma::Types::TopEdge:
        slideedge = WindowSystem::AbstractWindowInterface::Slide::Top;
        break;

    case Plasma::Types::RightEdge:
        slideedge = WindowSystem::AbstractWindowInterface::Slide::Right;
        break;

    case Plasma::Types::BottomEdge:
        slideedge = WindowSystem::AbstractWindowInterface::Slide::Bottom;
        break;

    case Plasma::Types::LeftEdge:
        slideedge = WindowSystem::AbstractWindowInterface::Slide::Left;
        break;

    default:
        qDebug() << staticMetaObject.className() << "wrong location";
        break;
    }

    return slideedge;
}

void Positioner::slideOutDuringExit(Plasma::Types::Location location)
{
    if (m_view->isVisible()) {
        m_corona->wm()->slideWindow(*m_view, slideLocation(location));
        m_view->setVisible(false);
    }
}

void Positioner::slideInDuringStartup()
{
    m_corona->wm()->slideWindow(*m_view, slideLocation(m_view->containment()->location()));
}

void Positioner::onStartupFinished()
{
    if (m_inStartup) {
        m_inStartup = false;
        syncGeometry();
        emit isOffScreenChanged();
    }
}

void Positioner::onCurrentLayoutIsSwitching(const QString &layoutName)
{
    if (!m_view || !m_view->layout() || m_view->layout()->name() != layoutName || !m_view->isVisible()) {
        return;
    }

    m_inLayoutUnloading = true;
    slideOutDuringExit();
}

void Positioner::setWindowOnActivities(const Latte::WindowSystem::WindowId &wid, const QStringList &activities)
{
    m_corona->wm()->setWindowOnActivities(wid, activities);
}

void Positioner::syncLatteViews()
{
    if (m_view->layout()) {
        //! This is needed in case the edge there are views that must be deleted
        //! after screen edges changes
        m_view->layout()->syncLatteViewsToScreens();
    }
}

void Positioner::updateContainmentScreen()
{
    if (m_view->containment()) {
        m_view->containment()->reactToScreenChange();
    }
}

//! this function updates the dock's associated screen.
//! updateScreenId = true, update also the m_screenNameToFollow
//! updateScreenId = false, do not update the m_screenNameToFollow
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
        m_screenNameToFollow = scr->name();
    }

    qDebug() << "adapting to screen...";
    m_view->setScreen(scr);

    updateContainmentScreen();

    connect(scr, &QScreen::geometryChanged, this, &Positioner::screenGeometryChanged);
    syncGeometry();
    m_view->updateAbsoluteGeometry(true);
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

    for (const auto scr : qGuiApp->screens()) {
        qDebug() << "      D, found screen: " << scr->name();
    }

    bool screenExists{false};
    QScreen *primaryScreen{m_corona->screenPool()->primaryScreen()};

    //!check if the associated screen is running
    for (const auto scr : qGuiApp->screens()) {
        if (m_screenNameToFollow == scr->name()
                || (m_view->onPrimary() && scr == primaryScreen)) {
            screenExists = true;
        }
    }

    qDebug() << "dock screen exists  ::: " << screenExists;

    //! 1.a primary dock must be always on the primary screen
    if (m_view->onPrimary() && (m_screenNameToFollow != primaryScreen->name()
                                || m_screenToFollow != primaryScreen
                                || m_view->screen() != primaryScreen)) {
        //! case 1
        qDebug() << "reached case 1: of updating dock primary screen...";
        setScreenToFollow(primaryScreen);
    } else if (!m_view->onPrimary()) {
        //! 2.an explicit dock must be always on the correct associated screen
        //! there are cases that window manager misplaces the dock, this function
        //! ensures that this dock will return at its correct screen
        for (const auto scr : qGuiApp->screens()) {
            if (scr && scr->name() == m_screenNameToFollow) {
                qDebug() << "reached case 2: updating the explicit screen for dock...";
                setScreenToFollow(scr);
                break;
            }
        }
    }

    syncGeometry();
    qDebug() << "reconsiderScreen() ended...";
}

void Positioner::onScreenChanged(QScreen *scr)
{
    m_screenSyncTimer.start();

    //! this is needed in order to update the struts on screen change
    //! and even though the geometry has been set correctly the offsets
    //! of the screen must be updated to the new ones
    if (m_view->visibility() && m_view->visibility()->mode() == Latte::Types::AlwaysVisible) {
        m_view->updateAbsoluteGeometry(true);
    }
}

void Positioner::syncGeometry()
{
    if (!(m_view->screen() && m_view->containment()) || m_inDelete || m_slideOffset!=0 || inSlideAnimation()) {
        return;
    }

    qDebug() << "syncGeometry() called...";

    if (!m_syncGeometryTimer.isActive()) {
        m_syncGeometryTimer.start();
    }
}

void Positioner::immediateSyncGeometry()
{
    bool found{false};

    qDebug() << "immediateSyncGeometry() called...";

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
        QRect availableScreenRect = m_view->screen()->geometry();

        if (m_inStartup) {
            //! paint out-of-screen
            availableScreenRect = QRect(-9999, -9999, m_view->screen()->geometry().width(), m_view->screen()->geometry().height());
        }

        if (m_view->formFactor() == Plasma::Types::Vertical) {
            QString layoutName = m_view->layout() ? m_view->layout()->name() : QString();
            auto latteCorona = qobject_cast<Latte::Corona *>(m_view->corona());
            int fixedScreen = m_view->onPrimary() ? latteCorona->screenPool()->primaryScreenId() : m_view->containment()->screen();

            QList<Types::Visibility> ignoreModes({Latte::Types::AutoHide,
                                                  Latte::Types::SidebarOnDemand,
                                                  Latte::Types::SidebarAutoHide});

            QList<Plasma::Types::Location> ignoreEdges({Plasma::Types::LeftEdge,
                                                        Plasma::Types::RightEdge});

            if (m_isStickedOnTopEdge && m_isStickedOnBottomEdge) {
                //! dont send an empty edges array because that means include all screen edges in calculations
                ignoreEdges << Plasma::Types::TopEdge;
                ignoreEdges << Plasma::Types::BottomEdge;
            } else {
                if (m_isStickedOnTopEdge) {
                    ignoreEdges << Plasma::Types::TopEdge;
                }

                if (m_isStickedOnBottomEdge) {
                    ignoreEdges << Plasma::Types::BottomEdge;
                }
            }

            QString activityid = m_view->layout() ? m_view->layout()->lastUsedActivity() : QString();
            if (m_inStartup) {
                //! paint out-of-screen
                freeRegion = availableScreenRect;
            } else {
                freeRegion = latteCorona->availableScreenRegionWithCriteria(fixedScreen, activityid, ignoreModes, ignoreEdges);
            }

            //! On startup when offscreen use offscreen screen geometry.
            //! This way vertical docks and panels are not showing are shrinked that
            //! need to be expanded after sliding-in in startup
            maximumRect = maximumNormalGeometry(m_inStartup ? availableScreenRect : QRect());
            QRegion availableRegion = freeRegion.intersected(maximumRect);

            availableScreenRect = freeRegion.intersected(maximumRect).boundingRect();
            float area = 0;

            //! it is used to choose which or the availableRegion rectangles will
            //! be the one representing dock geometry
            for (QRegion::const_iterator p_rect=availableRegion.begin(); p_rect!=availableRegion.end(); ++p_rect) {
                //! the area of each rectangle in calculated in squares of 50x50
                //! this is a way to avoid enourmous numbers for area value
                float tempArea = (float)((*p_rect).width() * (*p_rect).height()) / 2500;

                if (tempArea > area) {
                    availableScreenRect = (*p_rect);
                    area = tempArea;
                }
            }

            validateTopBottomBorders(availableScreenRect, freeRegion);
            m_lastAvailableScreenRegion = freeRegion;
        } else {
            m_view->effects()->setForceTopBorder(false);
            m_view->effects()->setForceBottomBorder(false);
        }

        m_lastAvailableScreenRect = availableScreenRect;

        m_view->effects()->updateEnabledBorders();

        resizeWindow(availableScreenRect);
        updatePosition(availableScreenRect);
        updateCanvasGeometry(availableScreenRect);

        qDebug() << "syncGeometry() calculations for screen: " << m_view->screen()->name() << " _ " << m_view->screen()->geometry();
        qDebug() << "syncGeometry() calculations for edge: " << m_view->location();
    }

    qDebug() << "syncGeometry() ended...";

    // qDebug() << "dock geometry:" << qRectToStr(geometry());
}

void Positioner::validateDockGeometry()
{
    if (m_slideOffset==0 && m_view->geometry() != m_validGeometry) {
        m_validateGeometryTimer.start();
    }
}

QRect Positioner::canvasGeometry()
{
    return m_canvasGeometry;
}

void Positioner::setCanvasGeometry(const QRect &geometry)
{
    if (m_canvasGeometry == geometry) {
        return;
    }

    m_canvasGeometry = geometry;
    emit canvasGeometryChanged();
}


//! this is used mainly from vertical panels in order to
//! to get the maximum geometry that can be used from the dock
//! based on their alignment type and the location dock
QRect Positioner::maximumNormalGeometry(QRect screenGeometry)
{
    QRect currentScrGeometry = screenGeometry.isEmpty() ? m_view->screen()->geometry() : screenGeometry;

    int xPos = 0;
    int yPos = currentScrGeometry.y();;
    int maxHeight = currentScrGeometry.height();
    int maxWidth = m_view->maxNormalThickness();
    QRect maxGeometry;
    maxGeometry.setRect(0, 0, maxWidth, maxHeight);

    switch (m_view->location()) {
    case Plasma::Types::LeftEdge:
        xPos = currentScrGeometry.x();
        maxGeometry.setRect(xPos, yPos, maxWidth, maxHeight);
        break;

    case Plasma::Types::RightEdge:
        xPos = currentScrGeometry.right() - maxWidth + 1;
        maxGeometry.setRect(xPos, yPos, maxWidth, maxHeight);
        break;

    default:
        //! bypass clang warnings
        break;
    }

    return maxGeometry;
}

void Positioner::validateTopBottomBorders(QRect availableScreenRect, QRegion availableScreenRegion)
{
    //! Check if the the top/bottom borders must be drawn also
    int edgeMargin = qMax(1, m_view->screenEdgeMargin());

    if (availableScreenRect.top() != m_view->screenGeometry().top()) {
        //! check top border
        int x = m_view->location() == Plasma::Types::LeftEdge ? m_view->screenGeometry().x() : m_view->screenGeometry().right() - edgeMargin + 1;
        QRegion fitInRegion = QRect(x, availableScreenRect.y()-1, edgeMargin, 1);
        QRegion subtracted = fitInRegion.subtracted(availableScreenRegion);

        if (subtracted.isNull()) {
            //!FitIn rectangle fits TOTALLY in the free screen region and as such
            //!the top border should be drawn
            m_view->effects()->setForceTopBorder(true);
        } else {
            m_view->effects()->setForceTopBorder(false);
        }
    } else {
        m_view->effects()->setForceTopBorder(false);
    }

    if (availableScreenRect.bottom() != m_view->screenGeometry().bottom()) {
        //! check top border
        int x = m_view->location() == Plasma::Types::LeftEdge ? m_view->screenGeometry().x() : m_view->screenGeometry().right()  - edgeMargin + 1;
        QRegion fitInRegion = QRect(x, availableScreenRect.bottom()+1, edgeMargin, 1);
        QRegion subtracted = fitInRegion.subtracted(availableScreenRegion);

        if (subtracted.isNull()) {
            //!FitIn rectangle fits TOTALLY in the free screen region and as such
            //!the BOTTOM border should be drawn
            m_view->effects()->setForceBottomBorder(true);
        } else {
            m_view->effects()->setForceBottomBorder(false);
        }
    } else {
        m_view->effects()->setForceBottomBorder(false);
    }
}

void Positioner::updateCanvasGeometry(QRect availableScreenRect)
{
    if (availableScreenRect.isEmpty()) {
        return;
    }

    QRect canvas;
    QRect screenGeometry{m_view->screen()->geometry()};
    int thickness{m_view->editThickness()};

    if (m_view->formFactor() == Plasma::Types::Vertical) {
        canvas.setWidth(thickness);
        canvas.setHeight(availableScreenRect.height());
    } else {
        canvas.setWidth(screenGeometry.width());
        canvas.setHeight(thickness);
    }

    switch (m_view->location()) {
    case Plasma::Types::TopEdge:
        canvas.moveLeft(screenGeometry.x());
        canvas.moveTop(screenGeometry.y());
        break;

    case Plasma::Types::BottomEdge:
        canvas.moveLeft(screenGeometry.x());
        canvas.moveTop(screenGeometry.bottom() - thickness + 1);
        break;

    case Plasma::Types::RightEdge:
        canvas.moveLeft(screenGeometry.right() - thickness + 1);
        canvas.moveTop(availableScreenRect.y());
        break;

    case Plasma::Types::LeftEdge:
        canvas.moveLeft(availableScreenRect.x());
        canvas.moveTop(availableScreenRect.y());
        break;

    default:
        qWarning() << "wrong location, couldn't update the canvas config window geometry " << m_view->location();
    }

    setCanvasGeometry(canvas);
}

void Positioner::updatePosition(QRect availableScreenRect)
{
    QRect screenGeometry{availableScreenRect};
    QPoint position;
    position = {0, 0};

    const auto gap = [&](int scr_length) -> int {
        return static_cast<int>(scr_length * m_view->offset());
    };
    const auto gapCentered = [&](int scr_length) -> int {
        return static_cast<int>(scr_length * ((1 - m_view->maxLength()) / 2) + scr_length * m_view->offset());
    };
    const auto gapReversed = [&](int scr_length) -> int {
        return static_cast<int>(scr_length - (scr_length * m_view->maxLength()) - gap(scr_length));
    };

    int cleanThickness = m_view->normalThickness() - m_view->effects()->innerShadow();

    int screenEdgeMargin = m_view->behaveAsPlasmaPanel() ? m_view->screenEdgeMargin() - qAbs(m_slideOffset) : 0;

    switch (m_view->location()) {
    case Plasma::Types::TopEdge:
        if (m_view->behaveAsPlasmaPanel()) {
            int y = screenGeometry.y() + screenEdgeMargin;

            if (m_view->alignment() == Latte::Types::Left) {
                position = {screenGeometry.x() + gap(screenGeometry.width()), y};
            } else if (m_view->alignment() == Latte::Types::Right) {
                position = {screenGeometry.x() + gapReversed(screenGeometry.width()) + 1, y};
            } else {
                position = {screenGeometry.x() + gapCentered(screenGeometry.width()), y};
            }
        } else {
            position = {screenGeometry.x(), screenGeometry.y()};
        }

        break;

    case Plasma::Types::BottomEdge:
        if (m_view->behaveAsPlasmaPanel()) {
            int y = screenGeometry.y() + screenGeometry.height() - cleanThickness - screenEdgeMargin;

            if (m_view->alignment() == Latte::Types::Left) {
                position = {screenGeometry.x() + gap(screenGeometry.width()), y};
            } else if (m_view->alignment() == Latte::Types::Right) {
                position = {screenGeometry.x() + gapReversed(screenGeometry.width()) + 1, y};
            } else {
                position = {screenGeometry.x() + gapCentered(screenGeometry.width()), y};
            }
        } else {
            position = {screenGeometry.x(), screenGeometry.y() + screenGeometry.height() - m_view->height()};
        }

        break;

    case Plasma::Types::RightEdge:
        if (m_view->behaveAsPlasmaPanel()) {
            int x = availableScreenRect.right() - cleanThickness + 1 - screenEdgeMargin;

            if (m_view->alignment() == Latte::Types::Top) {
                position = {x, availableScreenRect.y() + gap(availableScreenRect.height())};
            } else if (m_view->alignment() == Latte::Types::Bottom) {
                position = {x, availableScreenRect.y() + gapReversed(availableScreenRect.height()) + 1};
            } else {
                position = {x, availableScreenRect.y() + gapCentered(availableScreenRect.height())};
            }
        } else {
            position = {availableScreenRect.right() - m_view->width() + 1, availableScreenRect.y()};
        }

        break;

    case Plasma::Types::LeftEdge:
        if (m_view->behaveAsPlasmaPanel()) {
            int x = availableScreenRect.x() + screenEdgeMargin;

            if (m_view->alignment() == Latte::Types::Top) {
                position = {x, availableScreenRect.y() + gap(availableScreenRect.height())};
            } else if (m_view->alignment() == Latte::Types::Bottom) {
                position = {x, availableScreenRect.y() + gapReversed(availableScreenRect.height()) + 1};
            } else {
                position = {x, availableScreenRect.y() + gapCentered(availableScreenRect.height())};
            }
        } else {
            position = {availableScreenRect.x(), availableScreenRect.y()};
        }

        break;

    default:
        qWarning() << "wrong location, couldn't update the panel position"
                   << m_view->location();
    }

    if (m_slideOffset == 0 || m_nextScreenEdge != Plasma::Types::Floating /*exactly after relocating and changing screen edge*/) {
        //! update valid geometry in normal positioning
        m_validGeometry.moveTopLeft(position);
    } else {
        //! when sliding in/out update only the relevant axis for the screen_edge in
        //! to not mess the calculations and the automatic geometry checkers that
        //! View::Positioner is using.
        if (m_view->formFactor() == Plasma::Types::Horizontal) {
            m_validGeometry.moveLeft(position.x());
        } else {
            m_validGeometry.moveTop(position.y());
        }
    }

    m_view->setPosition(position);

    if (m_view->surface()) {
        m_view->surface()->setPosition(position);
    }
}

int Positioner::slideOffset() const
{
    return m_slideOffset;
}

void Positioner::setSlideOffset(int offset)
{
    if (m_slideOffset == offset) {
        return;
    }

    m_slideOffset = offset;
    emit slideOffsetChanged();
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

    //! protect from invalid window size under wayland
    size.setWidth(qMax(1, size.width()));
    size.setHeight(qMax(1, size.height()));

    m_validGeometry.setSize(size);

    m_view->setMinimumSize(size);
    m_view->setMaximumSize(size);
    m_view->resize(size);

    if (m_view->formFactor() == Plasma::Types::Horizontal) {
        emit windowSizeChanged();
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

void Positioner::onLastRepositionApplyEvent()
{
    m_view->effects()->setAnimationsBlocked(false);
    setInRelocationShowing(true);
    emit showingAfterRelocationFinished();
    emit edgeChanged();

    if (m_repositionFromViewSettingsWindow) {
        m_repositionFromViewSettingsWindow = false;
        m_view->showSettingsWindow();
    }
}

void Positioner::initSignalingForLocationChangeSliding()
{
    connect(this, &Positioner::hidingForRelocationStarted, this, &Positioner::onHideWindowsForSlidingOut);

    //! SCREEN_EDGE
    connect(m_view, &View::locationChanged, this, [&]() {
        if (m_nextScreenEdge != Plasma::Types::Floating) {
            bool isrelocationlastevent = isLastHidingRelocationEvent();
            immediateSyncGeometry();
            m_nextScreenEdge = Plasma::Types::Floating;

            //! make sure that View has been repositioned properly in next screen edge and show view afterwards
            if (isrelocationlastevent) {
                QTimer::singleShot(100, [this]() {
                    onLastRepositionApplyEvent();
                });
            }
        }
    });

    //! SCREEN
    connect(m_view, &QQuickView::screenChanged, this, [&]() {
        if (!m_view || !m_nextScreen) {
            return;
        }

        //[1] if panels are not excluded from confirmed geometry check then they are stuck in sliding out end
        //and they do not switch to new screen geometry
        //[2] under wayland view geometry may be delayed to be updated even though the screen has been updated correctly
        bool confirmedgeometry = KWindowSystem::isPlatformWayland() || m_view->behaveAsPlasmaPanel() || (!m_view->behaveAsPlasmaPanel() && m_nextScreen->geometry().contains(m_view->geometry().center()));

        if (m_nextScreen
                && m_nextScreen == m_view->screen()
                && confirmedgeometry) {
            bool isrelocationlastevent = isLastHidingRelocationEvent();
            m_nextScreen = nullptr;
            m_nextScreenName = "";

            //! make sure that View has been repositioned properly in next screen and show view afterwards
            if (isrelocationlastevent) {
                QTimer::singleShot(100, [this]() {
                    onLastRepositionApplyEvent();
                });
            }
        }
    });

    //! LAYOUT
    connect(m_view, &View::layoutChanged, this, [&]() {
        if (!m_nextLayoutName.isEmpty() && m_view->layout()) {
            bool isrelocationlastevent = isLastHidingRelocationEvent();
            m_nextLayoutName = "";

            //! make sure that View has been repositioned properly in next layout and show view afterwards
            if (isrelocationlastevent) {
                QTimer::singleShot(100, [this]() {
                    onLastRepositionApplyEvent();
                });
            }
        }
    });

    //! APPLY CHANGES
    connect(this, &Positioner::hidingForRelocationFinished, this, [&]() {
        //! must be called only if relocation is animated
        if (m_repositionIsAnimated) {
            m_repositionIsAnimated = false;
            m_view->effects()->setAnimationsBlocked(true);
        }

        //! LAYOUT
        if (!m_nextLayoutName.isEmpty()) {
            m_corona->layoutsManager()->moveView(m_view->layout()->name(), m_view->containment()->id(), m_nextLayoutName);
        }

        //! SCREEN
        if (!m_nextScreenName.isEmpty()) {
            bool nextonprimary = (m_nextScreenName == Latte::Data::Screen::ONPRIMARYNAME);
            m_nextScreen = m_corona->screenPool()->primaryScreen();

            if (!nextonprimary) {
                for (const auto scr : qGuiApp->screens()) {
                    if (scr && scr->name() == m_nextScreenName) {
                        m_nextScreen = scr;
                        break;
                    }
                }
            }

            m_view->setOnPrimary(nextonprimary);
            setScreenToFollow(m_nextScreen);
        }

        //! SCREEN_EDGE
        if (m_nextScreenEdge != Plasma::Types::Floating) {
            m_view->setLocation(m_nextScreenEdge);
        }

        //! ALIGNMENT
        if (m_nextAlignment != Latte::Types::NoneAlignment && m_nextAlignment != m_view->alignment()) {
            m_view->setAlignment(m_nextAlignment);
            m_nextAlignment = Latte::Types::NoneAlignment;
        }

        //! SCREENSGROUP
        if (m_view->isOriginal()) {
            auto originalview = qobject_cast<Latte::OriginalView *>(m_view);
            originalview->setScreensGroup(m_nextScreensGroup);
        }
    });
}

bool Positioner::inLayoutUnloading()
{
    return m_inLayoutUnloading;
}

bool Positioner::inRelocationAnimation()
{
    return ((m_nextScreenEdge != Plasma::Types::Floating) || !m_nextLayoutName.isEmpty() || !m_nextScreenName.isEmpty());
}

bool Positioner::inSlideAnimation() const
{
    return m_inSlideAnimation;
}

void Positioner::setInSlideAnimation(bool active)
{
    if (m_inSlideAnimation == active) {
        return;
    }

    m_inSlideAnimation = active;
    emit inSlideAnimationChanged();
}

bool Positioner::isCursorInsideView() const
{
    return m_view->geometry().contains(QCursor::pos(m_screenToFollow));
}

bool Positioner::isStickedOnTopEdge() const
{
    return m_isStickedOnTopEdge;
}

void Positioner::setIsStickedOnTopEdge(bool sticked)
{
    if (m_isStickedOnTopEdge == sticked) {
        return;
    }

    m_isStickedOnTopEdge = sticked;
    emit isStickedOnTopEdgeChanged();
}

bool Positioner::isStickedOnBottomEdge() const
{
    return m_isStickedOnBottomEdge;
}

void Positioner::setIsStickedOnBottomEdge(bool sticked)
{
    if (m_isStickedOnBottomEdge == sticked) {
        return;
    }

    m_isStickedOnBottomEdge = sticked;
    emit isStickedOnBottomEdgeChanged();
}

void Positioner::updateInRelocationAnimation()
{
    bool inrelocationanimation = inRelocationAnimation();

    if (m_inRelocationAnimation == inrelocationanimation) {
        return;
    }

    m_inRelocationAnimation = inrelocationanimation;
    emit inRelocationAnimationChanged();
}

bool Positioner::isLastHidingRelocationEvent() const
{
    int events{0};

    if (!m_nextLayoutName.isEmpty()) {
        events++;
    }

    if (!m_nextScreenName.isEmpty()){
        events++;
    }

    if (m_nextScreenEdge != Plasma::Types::Floating) {
        events++;
    }

    return (events <= 1);
}

void Positioner::setNextLocation(const QString layoutName, const int screensGroup, QString screenName, int edge, int alignment)
{
    bool isanimated{false};
    bool haschanges{false};

    //! LAYOUT
    if (!layoutName.isEmpty()) {
        auto layout = m_view->layout();
        auto origin = qobject_cast<CentralLayout *>(layout);
        auto destination = m_corona->layoutsManager()->synchronizer()->centralLayout(layoutName);

        if (origin && destination && origin!=destination) {
            //! Needs to be updated; when the next layout is in the same Visible Workarea
            //! with the old one changing layouts should be instant
            bool inVisibleWorkarea{origin->lastUsedActivity() == destination->lastUsedActivity()};

            haschanges = true;
            m_nextLayoutName = layoutName;

            if (!inVisibleWorkarea) {
                isanimated = true;
            }
        }
    }

    //! SCREENSGROUP
    if (m_view->isOriginal()) {
        auto originalview = qobject_cast<Latte::OriginalView *>(m_view);
        //!initialize screens group
        m_nextScreensGroup = originalview->screensGroup();

        if (m_nextScreensGroup != screensGroup) {
            haschanges = true;
            m_nextScreensGroup = static_cast<Latte::Types::ScreensGroup>(screensGroup);

            if (m_nextScreensGroup == Latte::Types::AllScreensGroup) {
                screenName = Latte::Data::Screen::ONPRIMARYNAME;
            } else if (m_nextScreensGroup == Latte::Types::AllSecondaryScreensGroup) {
                int scrid = originalview->expectedScreenIdFromScreenGroup(m_nextScreensGroup);

                if (scrid != Latte::ScreenPool::NOSCREENID) {
                    screenName = m_corona->screenPool()->connector(scrid);
                }
            }
        }
    } else {
        m_nextScreensGroup = Latte::Types::SingleScreenGroup;
    }

    //! SCREEN
    if (!screenName.isEmpty()) {
        bool nextonprimary = (screenName == Latte::Data::Screen::ONPRIMARYNAME);

        if ( (m_view->onPrimary() && !nextonprimary) /*primary -> explicit*/
             || (!m_view->onPrimary() && nextonprimary) /*explicit -> primary*/
             || (!m_view->onPrimary() && !nextonprimary && screenName != currentScreenName()) ) { /*explicit -> new_explicit*/

            QString nextscreenname = nextonprimary ? m_corona->screenPool()->primaryScreen()->name() : screenName;

            if (currentScreenName() == nextscreenname) {
                m_view->setOnPrimary(nextonprimary);
                updateContainmentScreen();
            } else {
                m_nextScreenName = screenName;
                isanimated = true;
                haschanges = true;
            }
        }
    }

    //! SCREEN_EDGE
    if (edge != Plasma::Types::Floating) {
        if (edge != m_view->location()) {
            m_nextScreenEdge = static_cast<Plasma::Types::Location>(edge);
            isanimated = true;
            haschanges = true;
        }
    }

    //! ALIGNMENT
    if (alignment != Latte::Types::NoneAlignment && m_view->alignment() != alignment) {
        m_nextAlignment = static_cast<Latte::Types::Alignment>(alignment);
        haschanges = true;
    }

    if (haschanges && m_view->isOriginal()) {
        auto originalview = qobject_cast<Latte::OriginalView *>(m_view);
        originalview->setNextLocationForClones(layoutName, edge, alignment);
    }

    m_repositionIsAnimated = isanimated;
    m_repositionFromViewSettingsWindow = m_view->settingsWindowIsShown();

    if (isanimated) {
        emit hidingForRelocationStarted();
    } else if (haschanges){
        emit hidingForRelocationFinished();
    }
}

}
}
