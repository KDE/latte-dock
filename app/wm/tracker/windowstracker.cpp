/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "windowstracker.h"

// local
#include "lastactivewindow.h"
#include "schemes.h"
#include "trackedlayoutinfo.h"
#include "trackedviewinfo.h"
#include "../abstractwindowinterface.h"
#include "../schemecolors.h"
#include "../../apptypes.h"
#include "../../lattecorona.h"
#include "../../layout/genericlayout.h"
#include "../../layouts/manager.h"
#include "../../view/view.h"
#include "../../view/positioner.h"

// Qt
#include <KWindowSystem>

namespace Latte {
namespace WindowSystem {
namespace Tracker {

Windows::Windows(AbstractWindowInterface *parent)
    : QObject(parent)
{
    m_wm = parent;

    m_extraViewHintsTimer.setInterval(600);
    m_extraViewHintsTimer.setSingleShot(true);

    connect(&m_extraViewHintsTimer, &QTimer::timeout, this, &Windows::updateExtraViewHints);

    //! delayed application data
    m_updateApplicationDataTimer.setInterval(1500);
    m_updateApplicationDataTimer.setSingleShot(true);
    connect(&m_updateApplicationDataTimer, &QTimer::timeout, this, &Windows::updateApplicationData);

    //! delayed update all hints
    m_updateAllHintsTimer.setInterval(300);
    m_updateAllHintsTimer.setSingleShot(true);
    connect(&m_updateAllHintsTimer, &QTimer::timeout, this, &Windows::updateAllHints);

    init();
}

Windows::~Windows()
{
    //! clear all the m_views tracking information
    for (QHash<Latte::View *, TrackedViewInfo *>::iterator i=m_views.begin(); i!=m_views.end(); ++i) {
        i.value()->deleteLater();
        m_views[i.key()] = nullptr;
    }

    m_views.clear();

    //! clear all the m_layouts tracking layouts
    for (QHash<Latte::Layout::GenericLayout *, TrackedLayoutInfo *>::iterator i=m_layouts.begin(); i!=m_layouts.end(); ++i) {
        i.value()->deleteLater();
        m_layouts[i.key()] = nullptr;
    }

    m_layouts.clear();
}

void Windows::init()
{
    connect(m_wm, &AbstractWindowInterface::windowChanged, this, [&](WindowId wid) {
        m_windows[wid] = m_wm->requestInfo(wid);
        updateAllHints();

        emit windowChanged(wid);
    });

    connect(m_wm, &AbstractWindowInterface::windowRemoved, this, [&](WindowId wid) {
        m_windows.remove(wid);

        //! application data
        m_initializedApplicationData.removeAll(wid);
        m_delayedApplicationData.removeAll(wid);

        updateAllHints();

        emit windowRemoved(wid);
    });

    connect(m_wm, &AbstractWindowInterface::windowAdded, this, [&](WindowId wid) {
        if (!m_windows.contains(wid)) {
            m_windows.insert(wid, m_wm->requestInfo(wid));
        }
        updateAllHints();
    });

    connect(m_wm, &AbstractWindowInterface::activeWindowChanged, this, [&](WindowId wid) {
        //! for some reason this is needed in order to update properly activeness values
        //! when the active window changes the previous active windows should be also updated
        for (const auto view : m_views.keys()) {
            WindowId lastWinId = m_views[view]->lastActiveWindow()->currentWinId();
            if ((lastWinId) != wid && m_windows.contains(lastWinId)) {
                m_windows[lastWinId] = m_wm->requestInfo(lastWinId);
            }
        }

        m_windows[wid] = m_wm->requestInfo(wid);
        updateAllHints();

        emit activeWindowChanged(wid);
    });

    connect(m_wm, &AbstractWindowInterface::currentDesktopChanged, this, &Windows::updateAllHints);
    connect(m_wm, &AbstractWindowInterface::currentActivityChanged,  this, &Windows::updateAllHints);    
    connect(m_wm, &AbstractWindowInterface::isShowingDesktopChanged,  this, &Windows::updateAllHints);
}

void Windows::initLayoutHints(Latte::Layout::GenericLayout *layout)
{
    if (!m_layouts.contains(layout)) {
        return;
    }

    setActiveWindowMaximized(layout, false);
    setExistsWindowActive(layout, false);
    setExistsWindowMaximized(layout, false);
    setActiveWindowScheme(layout, nullptr);
}

void Windows::initViewHints(Latte::View *view)
{
    if (!m_views.contains(view)) {
        return;
    }

    setActiveWindowMaximized(view, false);
    setActiveWindowTouching(view, false);
    setActiveWindowTouchingEdge(view, false);
    setExistsWindowActive(view, false);
    setExistsWindowTouching(view, false);
    setExistsWindowTouchingEdge(view, false);
    setExistsWindowMaximized(view, false);
    setIsTouchingBusyVerticalView(view, false);
    setActiveWindowScheme(view, nullptr);
    setTouchingWindowScheme(view, nullptr);
}

AbstractWindowInterface *Windows::wm()
{
    return m_wm;
}


void Windows::addView(Latte::View *view)
{
    if (m_views.contains(view)) {
        return;
    }

    m_views[view] = new TrackedViewInfo(this, view);

    updateScreenGeometries();

    //! Consider Layouts
    addRelevantLayout(view);

    connect(view, &Latte::View::layoutChanged, this, [&, view]() {
        addRelevantLayout(view);
    });

    connect(view, &Latte::View::screenGeometryChanged, this, &Windows::updateScreenGeometries);

    connect(view, &Latte::View::isTouchingBottomViewAndIsBusyChanged, this, &Windows::updateExtraViewHints);
    connect(view, &Latte::View::isTouchingTopViewAndIsBusyChanged, this, &Windows::updateExtraViewHints);
    connect(view, &Latte::View::absoluteGeometryChanged, this, &Windows::updateAllHintsAfterTimer);

    updateAllHints();

    emit informationAnnounced(view);
}

void Windows::removeView(Latte::View *view)
{
    if (!m_views.contains(view)) {
        return;
    }

    m_views[view]->deleteLater();
    m_views.remove(view);

    updateRelevantLayouts();
}

void Windows::addRelevantLayout(Latte::View *view)
{
    if (view->layout()) {
        bool initializing {false};

        if (!m_layouts.contains(view->layout())) {
            initializing = true;
            m_layouts[view->layout()] = new TrackedLayoutInfo(this, view->layout());
        }

        //! Update always the AllScreens tracking because there is a chance a view delayed to be assigned in a layout
        //! and that could create a state the AllScreens tracking will be disabled if there is a View requesting
        //! tracking and one that it does not during startup
        updateRelevantLayouts();

        if (initializing) {
            updateHints(view->layout());
            emit informationAnnouncedForLayout(view->layout());
        }
    }
}

void Windows::updateRelevantLayouts()
{
    QList<Latte::Layout::GenericLayout*> orphanedLayouts;

    //! REMOVE Orphaned Relevant layouts that have been removed or they don't contain any Views anymore
    for (QHash<Latte::Layout::GenericLayout *, TrackedLayoutInfo *>::iterator i=m_layouts.begin(); i!=m_layouts.end(); ++i) {
        bool hasView{false};
        for (QHash<Latte::View *, TrackedViewInfo *>::iterator j=m_views.begin(); j!=m_views.end(); ++j) {
            if (j.key() && i.key() && i.key() == j.key()->layout()) {
                hasView = true;
                break;
            }
        }

        if (!hasView)  {
            if (i.value()) {
                i.value()->deleteLater();
            }
            orphanedLayouts << i.key();
        }
    }

    for(const auto &layout : orphanedLayouts) {
        m_layouts.remove(layout);
    }

    //! UPDATE Enabled layout window tracking based on the Views that are requesting windows tracking
    for (QHash<Latte::Layout::GenericLayout *, TrackedLayoutInfo *>::iterator i=m_layouts.begin(); i!=m_layouts.end(); ++i) {
        bool hasViewEnabled{false};
        for (QHash<Latte::View *, TrackedViewInfo *>::iterator j=m_views.begin(); j!=m_views.end(); ++j) {
            if (i.key() == j.key()->layout() && j.value()->enabled()) {
                hasViewEnabled = true;
                break;
            }
        }

        if (i.value()) {
            i.value()->setEnabled(hasViewEnabled);

            if (!hasViewEnabled) {
                initLayoutHints(i.key());
            }
        }
    }
}

//! Views Properties And Hints

bool Windows::enabled(Latte::View *view)
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->enabled();
}

void Windows::setEnabled(Latte::View *view, const bool enabled)
{
    if (!m_views.contains(view) || m_views[view]->enabled() == enabled) {
        return;
    }

    m_views[view]->setEnabled(enabled);

    if (enabled) {
        updateHints(view);
    } else {
        initViewHints(view);
    }

    updateRelevantLayouts();

    emit enabledChanged(view);
}

bool Windows::activeWindowMaximized(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->activeWindowMaximized();
}

void Windows::setActiveWindowMaximized(Latte::View *view, bool activeMaximized)
{
    if (!m_views.contains(view) || m_views[view]->activeWindowMaximized() == activeMaximized) {
        return;
    }

    m_views[view]->setActiveWindowMaximized(activeMaximized);
    emit activeWindowMaximizedChanged(view);
}

bool Windows::activeWindowTouching(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->activeWindowTouching();
}

void Windows::setActiveWindowTouching(Latte::View *view, bool activeTouching)
{
    if (!m_views.contains(view) || m_views[view]->activeWindowTouching() == activeTouching) {
        return;
    }

    m_views[view]->setActiveWindowTouching(activeTouching);
    emit activeWindowTouchingChanged(view);
}

bool Windows::activeWindowTouchingEdge(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->activeWindowTouchingEdge();
}

void Windows::setActiveWindowTouchingEdge(Latte::View *view, bool activeTouchingEdge)
{
    if (!m_views.contains(view) || m_views[view]->activeWindowTouchingEdge() == activeTouchingEdge) {
        return;
    }

    m_views[view]->setActiveWindowTouchingEdge(activeTouchingEdge);
    emit activeWindowTouchingEdgeChanged(view);
}

bool Windows::existsWindowActive(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->existsWindowActive();
}

void Windows::setExistsWindowActive(Latte::View *view, bool windowActive)
{
    if (!m_views.contains(view) || m_views[view]->existsWindowActive() == windowActive) {
        return;
    }

    m_views[view]->setExistsWindowActive(windowActive);
    emit existsWindowActiveChanged(view);
}

bool Windows::existsWindowMaximized(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->existsWindowMaximized();
}

void Windows::setExistsWindowMaximized(Latte::View *view, bool windowMaximized)
{
    if (!m_views.contains(view) || m_views[view]->existsWindowMaximized() == windowMaximized) {
        return;
    }

    m_views[view]->setExistsWindowMaximized(windowMaximized);
    emit existsWindowMaximizedChanged(view);
}

bool Windows::existsWindowTouching(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->existsWindowTouching();
}

void Windows::setExistsWindowTouching(Latte::View *view, bool windowTouching)
{
    if (!m_views.contains(view) || m_views[view]->existsWindowTouching() == windowTouching) {
        return;
    }

    m_views[view]->setExistsWindowTouching(windowTouching);
    emit existsWindowTouchingChanged(view);
}

bool Windows::existsWindowTouchingEdge(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->existsWindowTouchingEdge();
}

void Windows::setExistsWindowTouchingEdge(Latte::View *view, bool windowTouchingEdge)
{
    if (!m_views.contains(view) || m_views[view]->existsWindowTouchingEdge() == windowTouchingEdge) {
        return;
    }

    m_views[view]->setExistsWindowTouchingEdge(windowTouchingEdge);
    emit existsWindowTouchingEdgeChanged(view);
}


bool Windows::isTouchingBusyVerticalView(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return false;
    }

    return m_views[view]->isTouchingBusyVerticalView();
}

void Windows::setIsTouchingBusyVerticalView(Latte::View *view, bool viewTouching)
{
    if (!m_views.contains(view) || m_views[view]->isTouchingBusyVerticalView() == viewTouching) {
        return;
    }

    m_views[view]->setIsTouchingBusyVerticalView(viewTouching);
    emit isTouchingBusyVerticalViewChanged(view);
}

SchemeColors *Windows::activeWindowScheme(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return nullptr;
    }

    return m_views[view]->activeWindowScheme();
}

void Windows::setActiveWindowScheme(Latte::View *view, WindowSystem::SchemeColors *scheme)
{
    if (!m_views.contains(view) || m_views[view]->activeWindowScheme() == scheme) {
        return;
    }

    m_views[view]->setActiveWindowScheme(scheme);
    emit activeWindowSchemeChanged(view);
}

SchemeColors *Windows::touchingWindowScheme(Latte::View *view) const
{
    if (!m_views.contains(view)) {
        return nullptr;
    }

    return m_views[view]->touchingWindowScheme();
}

void Windows::setTouchingWindowScheme(Latte::View *view, WindowSystem::SchemeColors *scheme)
{
    if (!m_views.contains(view) || m_views[view]->touchingWindowScheme() == scheme) {
        return;
    }

    m_views[view]->setTouchingWindowScheme(scheme);
    emit touchingWindowSchemeChanged(view);
}

LastActiveWindow *Windows::lastActiveWindow(Latte::View *view)
{
    if (!m_views.contains(view)) {
        return nullptr;
    }

    return m_views[view]->lastActiveWindow();
}

//! Layouts
bool Windows::enabled(Latte::Layout::GenericLayout *layout)
{
    if (!m_layouts.contains(layout)) {
        return false;
    }

    return m_layouts[layout]->enabled();
}

bool Windows::activeWindowMaximized(Latte::Layout::GenericLayout *layout) const
{
    if (!m_layouts.contains(layout)) {
        return false;
    }

    return m_layouts[layout]->activeWindowMaximized();
}

void Windows::setActiveWindowMaximized(Latte::Layout::GenericLayout *layout, bool activeMaximized)
{
    if (!m_layouts.contains(layout) || m_layouts[layout]->activeWindowMaximized() == activeMaximized) {
        return;
    }

    m_layouts[layout]->setActiveWindowMaximized(activeMaximized);
    emit activeWindowMaximizedChangedForLayout(layout);
}

bool Windows::existsWindowActive(Latte::Layout::GenericLayout *layout) const
{
    if (!m_layouts.contains(layout)) {
        return false;
    }

    return m_layouts[layout]->existsWindowActive();
}

void Windows::setExistsWindowActive(Latte::Layout::GenericLayout *layout, bool windowActive)
{
    if (!m_layouts.contains(layout) || m_layouts[layout]->existsWindowActive() == windowActive) {
        return;
    }

    m_layouts[layout]->setExistsWindowActive(windowActive);
    emit existsWindowActiveChangedForLayout(layout);
}

bool Windows::existsWindowMaximized(Latte::Layout::GenericLayout *layout) const
{
    if (!m_layouts.contains(layout)) {
        return false;
    }

    return m_layouts[layout]->existsWindowMaximized();
}

void Windows::setExistsWindowMaximized(Latte::Layout::GenericLayout *layout, bool windowMaximized)
{
    if (!m_layouts.contains(layout) || m_layouts[layout]->existsWindowMaximized() == windowMaximized) {
        return;
    }

    m_layouts[layout]->setExistsWindowMaximized(windowMaximized);
    emit existsWindowMaximizedChangedForLayout(layout);
}

SchemeColors *Windows::activeWindowScheme(Latte::Layout::GenericLayout *layout) const
{
    if (!m_layouts.contains(layout)) {
        return nullptr;
    }

    return m_layouts[layout]->activeWindowScheme();
}

void Windows::setActiveWindowScheme(Latte::Layout::GenericLayout *layout, WindowSystem::SchemeColors *scheme)
{
    if (!m_layouts.contains(layout) || m_layouts[layout]->activeWindowScheme() == scheme) {
        return;
    }

    m_layouts[layout]->setActiveWindowScheme(scheme);
    emit activeWindowSchemeChangedForLayout(layout);
}

LastActiveWindow *Windows::lastActiveWindow(Latte::Layout::GenericLayout *layout)
{
    if (!m_layouts.contains(layout)) {
        return nullptr;
    }

    return m_layouts[layout]->lastActiveWindow();
}


//! Windows
bool Windows::isValidFor(const WindowId &wid) const
{
    if (!m_windows.contains(wid)) {
        return false;
    }

    return m_windows[wid].isValid();
}

QIcon Windows::iconFor(const WindowId &wid)
{
    if (!m_windows.contains(wid)) {
        return QIcon();
    }

    if (m_windows[wid].icon().isNull()) {
        AppData data = m_wm->appDataFor(wid);

        QIcon icon = data.icon;

        if (icon.isNull()) {
            icon = m_wm->iconFor(wid);
        }

        m_windows[wid].setIcon(icon);
        return icon;
    }

    return m_windows[wid].icon();
}

QString Windows::appNameFor(const WindowId &wid)
{
    if (!m_windows.contains(wid)) {
        return QString();
    }

    if(!m_initializedApplicationData.contains(wid) && !m_delayedApplicationData.contains(wid)) {
        m_delayedApplicationData.append(wid);
        m_updateApplicationDataTimer.start();
    }

    if (m_windows[wid].appName().isEmpty()) {
        AppData data = m_wm->appDataFor(wid);

        m_windows[wid].setAppName(data.name);

        return data.name;
    }

    return m_windows[wid].appName();
}

void Windows::updateApplicationData()
{
    if (m_delayedApplicationData.count() > 0) {
        for(int i=0; i<m_delayedApplicationData.count(); ++i) {
            auto wid = m_delayedApplicationData[i];

            if (m_windows.contains(wid)) {
                AppData data = m_wm->appDataFor(wid);

                QIcon icon = data.icon;

                if (icon.isNull()) {
                    icon = m_wm->iconFor(wid);
                }

                m_windows[wid].setIcon(icon);
                m_windows[wid].setAppName(data.name);

                m_initializedApplicationData.append(wid);

                emit applicationDataChanged(wid);
            }
        }
    }

    m_delayedApplicationData.clear();
}

WindowInfoWrap Windows::infoFor(const WindowId &wid) const
{
    if (!m_windows.contains(wid)) {
        return WindowInfoWrap();
    }

    return m_windows[wid];
}



//! Windows Criteria Functions
bool Windows::intersects(Latte::View *view, const WindowInfoWrap &winfo)
{
    return (!winfo.isMinimized() && !winfo.isShaded() && winfo.geometry().intersects(view->absoluteGeometry()));
}

bool Windows::isActive(const WindowInfoWrap &winfo)
{
    return (winfo.isValid() && winfo.isActive() && !winfo.isMinimized());
}

bool Windows::isActiveInViewScreen(Latte::View *view, const WindowInfoWrap &winfo)
{
    auto screenGeometry = m_views[view]->screenGeometry();

    if (KWindowSystem::isPlatformX11() && view->devicePixelRatio() != 1.0) {
        //!Fix for X11 Global Scale, I dont think this could be pixel perfect accurate
        auto factor = view->devicePixelRatio();
        screenGeometry = QRect(qRound(screenGeometry.x() * factor),
                               qRound(screenGeometry.y() * factor),
                               qRound(screenGeometry.width() * factor),
                               qRound(screenGeometry.height() * factor));
    }

    return (winfo.isValid()
            && winfo.isActive()
            && !winfo.isMinimized()
            && screenGeometry.intersects(winfo.geometry()));
}

bool Windows::isMaximizedInViewScreen(Latte::View *view, const WindowInfoWrap &winfo)
{
    auto screenGeometry = m_views[view]->screenGeometry();

    if (KWindowSystem::isPlatformX11() && view->devicePixelRatio() != 1.0) {
        //!Fix for X11 Global Scale, I dont think this could be pixel perfect accurate
        auto factor = view->devicePixelRatio();
        screenGeometry = QRect(qRound(screenGeometry.x() * factor),
                               qRound(screenGeometry.y() * factor),
                               qRound(screenGeometry.width() * factor),
                               qRound(screenGeometry.height() * factor));
    }

    //! updated implementation to identify the screen that the maximized window is present
    //! in order to avoid: https://bugs.kde.org/show_bug.cgi?id=397700
    return (winfo.isValid()
            && !winfo.isMinimized()
            && !winfo.isShaded()
            && winfo.isMaximized()
            && screenGeometry.intersects(winfo.geometry()));
}

bool Windows::isTouchingView(Latte::View *view, const WindowSystem::WindowInfoWrap &winfo)
{
    return (winfo.isValid() && intersects(view, winfo));
}

bool Windows::isTouchingViewEdge(Latte::View *view, const QRect &windowgeometry)
{
    if (!view) {
        return false;
    }

    bool inViewThicknessEdge{false};
    bool inViewLengthBoundaries{false};

    QRect screenGeometry = view->screenGeometry();

    if (KWindowSystem::isPlatformX11() && view->devicePixelRatio() != 1.0) {
        //!Fix for X11 Global Scale, I dont think this could be pixel perfect accurate
        auto factor = view->devicePixelRatio();
        screenGeometry = QRect(qRound(screenGeometry.x() * factor),
                               qRound(screenGeometry.y() * factor),
                               qRound(screenGeometry.width() * factor),
                               qRound(screenGeometry.height() * factor));
    }

    bool inCurrentScreen{screenGeometry.contains(windowgeometry.topLeft()) || screenGeometry.contains(windowgeometry.bottomRight())};

    if (inCurrentScreen) {
        if (view->location() == Plasma::Types::TopEdge) {
            inViewThicknessEdge = (windowgeometry.y() == view->absoluteGeometry().bottom() + 1);
        } else if (view->location() == Plasma::Types::BottomEdge) {
            inViewThicknessEdge = (windowgeometry.bottom() == view->absoluteGeometry().top() - 1);
        } else if (view->location() == Plasma::Types::LeftEdge) {
            inViewThicknessEdge = (windowgeometry.x() == view->absoluteGeometry().right() + 1);
        } else if (view->location() == Plasma::Types::RightEdge) {
            inViewThicknessEdge = (windowgeometry.right() == view->absoluteGeometry().left() - 1);
        }

        if (view->formFactor() == Plasma::Types::Horizontal) {
            int yCenter = view->absoluteGeometry().center().y();

            QPoint leftChecker(windowgeometry.left(), yCenter);
            QPoint rightChecker(windowgeometry.right(), yCenter);

            bool fulloverlap = (windowgeometry.left()<=view->absoluteGeometry().left()) && (windowgeometry.right()>=view->absoluteGeometry().right());

            inViewLengthBoundaries = fulloverlap || view->absoluteGeometry().contains(leftChecker) || view->absoluteGeometry().contains(rightChecker);
        } else if (view->formFactor() == Plasma::Types::Vertical) {
            int xCenter = view->absoluteGeometry().center().x();

            QPoint topChecker(xCenter, windowgeometry.top());
            QPoint bottomChecker(xCenter, windowgeometry.bottom());

            bool fulloverlap = (windowgeometry.top()<=view->absoluteGeometry().top()) && (windowgeometry.bottom()>=view->absoluteGeometry().bottom());

            inViewLengthBoundaries = fulloverlap || view->absoluteGeometry().contains(topChecker) || view->absoluteGeometry().contains(bottomChecker);
        }
    }

    return (inViewThicknessEdge && inViewLengthBoundaries);
}

bool Windows::isTouchingViewEdge(Latte::View *view, const WindowInfoWrap &winfo)
{
    if (winfo.isValid() &&  !winfo.isMinimized()) {
        return isTouchingViewEdge(view, winfo.geometry());
    }

    return false;
}

void Windows::cleanupFaultyWindows()
{
    for (const auto &key : m_windows.keys()) {
        auto winfo = m_windows[key];

        //! garbage windows removing
        if (winfo.wid()<=0 || winfo.geometry() == QRect(0, 0, 0, 0)) {
            //qDebug() << "Faulty Geometry ::: " << winfo.wid();
            m_windows.remove(key);
        }
    }
}


void Windows::updateScreenGeometries()
{
    for (const auto view : m_views.keys()) {
        if (m_views[view]->screenGeometry() != view->screenGeometry()) {
            m_views[view]->setScreenGeometry(view->screenGeometry());

            if (m_views[view]->enabled()) {
                updateHints(view);
            }
        }
    }
}

void Windows::updateAllHintsAfterTimer()
{
    if (!m_updateAllHintsTimer.isActive()) {
        updateAllHints();
        m_updateAllHintsTimer.start();
    }
}

void Windows::updateAllHints()
{
    for (const auto view : m_views.keys()) {
        updateHints(view);
    }

    for (const auto layout : m_layouts.keys()) {
        updateHints(layout);
    }

    if (!m_extraViewHintsTimer.isActive()) {
        m_extraViewHintsTimer.start();
    }
}

void Windows::updateExtraViewHints()
{
    for (const auto horView : m_views.keys()) {
        if (!m_views.contains(horView) || !m_views[horView]->enabled() || !m_views[horView]->isTrackingCurrentActivity()) {
            continue;
        }

        if (horView->formFactor() == Plasma::Types::Horizontal) {
            bool touchingBusyVerticalView{false};

            for (const auto verView : m_views.keys()) {
                if (!m_views.contains(verView) || !m_views[verView]->enabled() || !m_views[verView]->isTrackingCurrentActivity()) {
                    continue;
                }

                bool sameScreen = (verView->positioner()->currentScreenId() == horView->positioner()->currentScreenId());

                if (verView->formFactor() == Plasma::Types::Vertical && sameScreen) {
                    bool hasEdgeTouch = isTouchingViewEdge(horView, verView->absoluteGeometry());

                    bool topTouch = horView->location() == Plasma::Types::TopEdge && verView->isTouchingTopViewAndIsBusy() && hasEdgeTouch;
                    bool bottomTouch = horView->location() == Plasma::Types::BottomEdge && verView->isTouchingBottomViewAndIsBusy() && hasEdgeTouch;

                    if (topTouch || bottomTouch) {
                        touchingBusyVerticalView = true;
                        break;
                    }
                }
            }

            //qDebug() << " Touching Busy Vertical View :: " << horView->location() << " - " << horView->positioner()->currentScreenId() << " :: " << touchingBusyVerticalView;

            setIsTouchingBusyVerticalView(horView, touchingBusyVerticalView);
        }
    }
}

void Windows::updateHints(Latte::View *view)
{
    if (!m_views.contains(view) || !m_views[view]->enabled() || !m_views[view]->isTrackingCurrentActivity()) {
        return;
    }

    bool foundActive{false};
    bool foundActiveInCurScreen{false};
    bool foundActiveTouchInCurScreen{false};
    bool foundActiveEdgeTouchInCurScreen{false};
    bool foundTouchInCurScreen{false};
    bool foundTouchEdgeInCurScreen{false};
    bool foundMaximizedInCurScreen{false};

    bool foundActiveGroupTouchInCurScreen{false};

    //! the notification window is not sending a remove signal and creates windows of geometry (0x0 0,0),
    //! maybe a garbage collector here is a good idea!!!
    bool existsFaultyWindow{false};

    WindowId maxWinId;
    WindowId activeWinId;
    WindowId touchWinId;
    WindowId touchEdgeWinId;
    WindowId activeTouchWinId;
    WindowId activeTouchEdgeWinId;

    //qDebug() << " -- TRACKING REPORT (SCREEN)--";

    //! First Pass
    for (const auto &winfo : m_windows) {
        if (m_wm->isShowingDesktop()) {
            break;
        }

        if (!existsFaultyWindow && (winfo.wid()<=0 || winfo.geometry() == QRect(0, 0, 0, 0))) {
            existsFaultyWindow = true;
        }

        if ( !m_wm->inCurrentDesktopActivity(winfo)
             || m_wm->hasBlockedTracking(winfo.wid())
             || winfo.isMinimized()) {
            continue;
        }

        //qDebug() << " _ _ _ ";
        //qDebug() << "TRACKING | WINDOW INFO :: " << winfo.wid() << " _ " << winfo.appName() << " _ " << winfo.geometry() << " _ " << winfo.display();

        if (isActive(winfo)) {
            foundActive = true;
        }

        if (isActiveInViewScreen(view, winfo)) {
            foundActiveInCurScreen = true;
            activeWinId = winfo.wid();
        }

        //! Maximized windows flags
        if ((winfo.isActive() && isMaximizedInViewScreen(view, winfo)) //! active maximized windows have higher priority than the rest maximized windows
                || (!foundMaximizedInCurScreen && isMaximizedInViewScreen(view, winfo))) {
            foundMaximizedInCurScreen = true;
            maxWinId = winfo.wid();
        }

        //! Touching windows flags

        bool touchingViewEdge = isTouchingViewEdge(view, winfo);
        bool touchingView =  isTouchingView(view, winfo);

        if (touchingView) {
            if (winfo.isActive()) {
                foundActiveTouchInCurScreen = true;
                activeTouchWinId = winfo.wid();
            } else {
                foundTouchInCurScreen = true;
                touchWinId = winfo.wid();
            }
        }

        if (touchingViewEdge) {
            if (winfo.isActive()) {
                foundActiveEdgeTouchInCurScreen = true;
                activeTouchEdgeWinId = winfo.wid();
            } else {
                foundTouchEdgeInCurScreen = true;
                touchEdgeWinId = winfo.wid();
            }
        }

        //qDebug() << "TRACKING |       ACTIVE:"<< foundActive <<  " ACT_TOUCH_CUR_SCR:" << foundActiveTouchInCurScreen << " MAXIM:"<<foundMaximizedInCurScreen;
        //qDebug() << "TRACKING |       TOUCHING VIEW EDGE:"<< touchingViewEdge << " TOUCHING VIEW:" << foundTouchInCurScreen;
    }

    if (existsFaultyWindow) {
        cleanupFaultyWindows();
    }

    //! PASS 2
    if (!m_wm->isShowingDesktop() && foundActiveInCurScreen && !foundActiveTouchInCurScreen) {
        //! Second Pass to track also Child windows if needed

        //qDebug() << "Windows Array...";
        //for (const auto &winfo : m_windows) {
        //    qDebug() << " - " << winfo.wid() << " - " << winfo.isValid() << " - " << winfo.display() << " - " << winfo.geometry() << " parent : " << winfo.parentId();
        //}
        //qDebug() << " - - - - - ";

        WindowInfoWrap activeInfo = m_windows[activeWinId];
        WindowId mainWindowId = activeInfo.isChildWindow() ? activeInfo.parentId() : activeWinId;

        for (const auto &winfo : m_windows) {
            if (!m_wm->inCurrentDesktopActivity(winfo)
                    || m_wm->hasBlockedTracking(winfo.wid())
                    || winfo.isMinimized()) {
                continue;
            }

            bool inActiveGroup = (winfo.wid() == mainWindowId || winfo.parentId() == mainWindowId);

            //! consider only windows that belong to active window group meaning the main window
            //! and its children
            if (!inActiveGroup) {
                continue;
            }

            if (isTouchingView(view, winfo)) {
                foundActiveGroupTouchInCurScreen = true;
                break;
            }
        }
    }


    //! HACK: KWin Effects such as ShowDesktop have no way to be identified and as such
    //! create issues with identifying properly touching and maximized windows. BUT when
    //! they are enabled then NO ACTIVE window is found. This is a way to identify these
    //! effects trigerring and disable the touch flags.
    //! BUG: 404483
    //! Disabled because it has fault identifications, e.g. when a window is maximized and
    //! Latte or Plasma are showing their View settings
    //foundMaximizedInCurScreen = foundMaximizedInCurScreen && foundActive;
    //foundTouchInCurScreen = foundTouchInCurScreen && foundActive;

    //! assign flags
    setExistsWindowActive(view, foundActiveInCurScreen);
    setActiveWindowTouching(view, foundActiveTouchInCurScreen || foundActiveGroupTouchInCurScreen);
    setActiveWindowTouchingEdge(view, foundActiveEdgeTouchInCurScreen);
    setActiveWindowMaximized(view, (maxWinId.toInt()>0 && (maxWinId == activeTouchWinId || maxWinId == activeTouchEdgeWinId)));
    setExistsWindowMaximized(view, foundMaximizedInCurScreen);
    setExistsWindowTouching(view, (foundTouchInCurScreen || foundActiveTouchInCurScreen || foundActiveGroupTouchInCurScreen));
    setExistsWindowTouchingEdge(view, (foundActiveEdgeTouchInCurScreen || foundTouchEdgeInCurScreen));

    //! update color schemes for active and touching windows
    setActiveWindowScheme(view, (foundActiveInCurScreen ? m_wm->schemesTracker()->schemeForWindow(activeWinId) : nullptr));

    if (foundActiveTouchInCurScreen) {
        setTouchingWindowScheme(view, m_wm->schemesTracker()->schemeForWindow(activeTouchWinId));
    } else if (foundActiveEdgeTouchInCurScreen) {
        setTouchingWindowScheme(view, m_wm->schemesTracker()->schemeForWindow(activeTouchEdgeWinId));
    } else if (foundMaximizedInCurScreen) {
        setTouchingWindowScheme(view, m_wm->schemesTracker()->schemeForWindow(maxWinId));
    } else if (foundTouchInCurScreen) {
        setTouchingWindowScheme(view, m_wm->schemesTracker()->schemeForWindow(touchWinId));
    } else if (foundTouchEdgeInCurScreen) {
        setTouchingWindowScheme(view, m_wm->schemesTracker()->schemeForWindow(touchEdgeWinId));
    } else {
        setTouchingWindowScheme(view, nullptr);
    }

    //! update LastActiveWindow
    if (foundActiveInCurScreen) {
        m_views[view]->setActiveWindow(activeWinId);
    }

    //! Debug
    //qDebug() << "TRACKING |      _________ FINAL RESULTS ________";
    //qDebug() << "TRACKING | SCREEN: " << view->positioner()->currentScreenId() << " , EDGE:" << view->location() << " , ENABLED:" << enabled(view);
    //qDebug() << "TRACKING | activeWindowTouching: " << foundActiveTouchInCurScreen << " ,activeWindowMaximized: " << activeWindowMaximized(view);
    //qDebug() << "TRACKING | existsWindowActive: " << foundActiveInCurScreen << " , existsWindowMaximized:" << existsWindowMaximized(view)
    //         << " , existsWindowTouching:"<<existsWindowTouching(view);
    //qDebug() << "TRACKING | activeEdgeWindowTouch: " <<  activeWindowTouchingEdge(view) << " , existsEdgeWindowTouch:" << existsWindowTouchingEdge(view);
    //qDebug() << "TRACKING | existsActiveGroupTouching: " << foundActiveGroupTouchInCurScreen;
}

void Windows::updateHints(Latte::Layout::GenericLayout *layout) {
    if (!m_layouts.contains(layout) || !m_layouts[layout]->enabled() || !m_layouts[layout]->isTrackingCurrentActivity()) {
        return;
    }

    bool foundActive{false};
    bool foundActiveMaximized{false};
    bool foundMaximized{false};

    //! the notification window is not sending a remove signal and creates windows of geometry (0x0 0,0),
    //! maybe a garbage collector here is a good idea!!!
    bool existsFaultyWindow{false};

    WindowId activeWinId;
    WindowId maxWinId;

    for (const auto &winfo : m_windows) {
        if (m_wm->isShowingDesktop()) {
            break;
        }

        if (!existsFaultyWindow && (winfo.wid()<=0 || winfo.geometry() == QRect(0, 0, 0, 0))) {
            existsFaultyWindow = true;
        }

        if (!m_wm->inCurrentDesktopActivity(winfo)
                || m_wm->hasBlockedTracking(winfo.wid())
                || winfo.isMinimized()) {
            continue;
        }

        if (isActive(winfo)) {
            foundActive = true;
            activeWinId = winfo.wid();

            if (winfo.isMaximized() && !winfo.isMinimized()) {
                foundActiveMaximized = true;
                maxWinId = winfo.wid();
            }
        }

        if (!foundActiveMaximized && winfo.isMaximized() && !winfo.isMinimized()) {
            foundMaximized = true;
            maxWinId = winfo.wid();
        }

        //qDebug() << "window geometry ::: " << winfo.geometry();
    }

    if (existsFaultyWindow) {
        cleanupFaultyWindows();
    }

    //! HACK: KWin Effects such as ShowDesktop have no way to be identified and as such
    //! create issues with identifying properly touching and maximized windows. BUT when
    //! they are enabled then NO ACTIVE window is found. This is a way to identify these
    //! effects trigerring and disable the touch flags.
    //! BUG: 404483
    //! Disabled because it has fault identifications, e.g. when a window is maximized and
    //! Latte or Plasma are showing their View settings
    //foundMaximizedInCurScreen = foundMaximizedInCurScreen && foundActive;
    //foundTouchInCurScreen = foundTouchInCurScreen && foundActive;

    //! assign flags
    setExistsWindowActive(layout, foundActive);
    setActiveWindowMaximized(layout, foundActiveMaximized);
    setExistsWindowMaximized(layout, foundActiveMaximized || foundMaximized);

    //! update color schemes for active and touching windows
    setActiveWindowScheme(layout, (foundActive ? m_wm->schemesTracker()->schemeForWindow(activeWinId) : nullptr));

    //! update LastActiveWindow
    if (foundActive) {
        m_layouts[layout]->setActiveWindow(activeWinId);
    }

    //! Debug
    //qDebug() << " -- TRACKING REPORT (LAYOUT) --";
    //qDebug() << "TRACKING | LAYOUT: " << layout->name() << " , ENABLED:" << enabled(layout);
    //qDebug() << "TRACKING | existsActiveWindow: " << foundActive << " ,activeWindowMaximized: " << foundActiveMaximized;
    //qDebug() << "TRACKING | existsWindowMaximized: " << existsWindowMaximized(layout);
}

}
}
}
