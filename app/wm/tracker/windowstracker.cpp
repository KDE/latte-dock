/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "windowstracker.h"

// local
#include "lastactivewindow.h"
#include "schemes.h"
#include "trackedlayoutinfo.h"
#include "trackedviewinfo.h"
#include "../abstractwindowinterface.h"
#include "../schemecolors.h"
#include "../../lattecorona.h"
#include "../../layout/genericlayout.h"
#include "../../layouts/manager.h"
#include "../../view/view.h"
#include "../../view/positioner.h"
#include "../../../liblatte2/types.h"

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
    connect(m_wm->corona(), &Plasma::Corona::availableScreenRectChanged, this, &Windows::updateAvailableScreenGeometries);

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
            WindowId lastWinId = m_views[view]->lastActiveWindow()->winId();
            if ((lastWinId) != wid && m_windows.contains(lastWinId)) {
                m_windows[lastWinId] = m_wm->requestInfo(lastWinId);
            }
        }

        m_windows[wid] = m_wm->requestInfo(wid);
        updateAllHints();

        emit activeWindowChanged(wid);
    });

    connect(m_wm, &AbstractWindowInterface::currentDesktopChanged, this, [&] {
        updateAllHints();
    });

    connect(m_wm, &AbstractWindowInterface::currentActivityChanged, this, [&] {
        if (m_wm->corona()->layoutsManager()->memoryUsage() == Types::MultipleLayouts) {
            //! this is needed in MultipleLayouts because there is a chance that multiple
            //! layouts are providing different available screen geometries in different Activities
            updateAvailableScreenGeometries();
        }

        updateAllHints();
    });
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
    setExistsWindowActive(view, false);
    setExistsWindowTouching(view, false);
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

    updateAvailableScreenGeometries();

    //! Consider Layouts
    addRelevantLayout(view);

    connect(view, &Latte::View::layoutChanged, this, [&, view]() {
        addRelevantLayout(view);
    });

    connect(view, &Latte::View::isTouchingBottomViewAndIsBusy, this, &Windows::updateExtraViewHints);
    connect(view, &Latte::View::isTouchingTopViewAndIsBusy, this, &Windows::updateExtraViewHints);

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

    return m_windows[wid].isValid() && !m_windows[wid].isPlasmaDesktop();
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
    return (winfo.isValid() && winfo.isActive() && !winfo.isPlasmaDesktop() && !winfo.isMinimized());
}

bool Windows::isActiveInViewScreen(Latte::View *view, const WindowInfoWrap &winfo)
{
    return (winfo.isValid() && winfo.isActive() && !winfo.isPlasmaDesktop() &&  !winfo.isMinimized()
            && m_views[view]->availableScreenGeometry().contains(winfo.geometry().center()));
}

bool Windows::isMaximizedInViewScreen(Latte::View *view, const WindowInfoWrap &winfo)
{
 /*   auto viewIntersectsMaxVert = [&]() noexcept -> bool {
            return ((winfo.isMaxVert()
                     || (view->screen() && view->screen()->availableSize().height() <= winfo.geometry().height()))
                    && intersects(view, winfo));
};

    auto viewIntersectsMaxHoriz = [&]() noexcept -> bool {
            return ((winfo.isMaxHoriz()
                     || (view->screen() && view->screen()->availableSize().width() <= winfo.geometry().width()))
                    && intersects(view, winfo));
};*/

    //! updated implementation to identify the screen that the maximized window is present
    //! in order to avoid: https://bugs.kde.org/show_bug.cgi?id=397700
    return (winfo.isValid() && !winfo.isPlasmaDesktop() && !winfo.isMinimized()
            && !winfo.isShaded()
            && winfo.isMaximized()
            && m_views[view]->availableScreenGeometry().contains(winfo.geometry().center()));
}

bool Windows::isTouchingView(Latte::View *view, const WindowSystem::WindowInfoWrap &winfo)
{
    return (winfo.isValid() && !winfo.isPlasmaDesktop() && intersects(view, winfo));
}

bool Windows::isTouchingViewEdge(Latte::View *view, const WindowInfoWrap &winfo)
{
    if (winfo.isValid() && !winfo.isPlasmaDesktop() &&  !winfo.isMinimized()) {
        bool inViewThicknessEdge{false};
        bool inViewLengthBoundaries{false};

        QRect screenGeometry = view->screenGeometry();

        bool inCurrentScreen{screenGeometry.contains(winfo.geometry().topLeft()) || screenGeometry.contains(winfo.geometry().bottomRight())};

        if (inCurrentScreen) {
            if (view->location() == Plasma::Types::TopEdge) {
                inViewThicknessEdge = (winfo.geometry().y() == view->absoluteGeometry().bottom() + 1);
            } else if (view->location() == Plasma::Types::BottomEdge) {
                inViewThicknessEdge = (winfo.geometry().bottom() == view->absoluteGeometry().top() - 1);
            } else if (view->location() == Plasma::Types::LeftEdge) {
                inViewThicknessEdge = (winfo.geometry().x() == view->absoluteGeometry().right() + 1);
            } else if (view->location() == Plasma::Types::RightEdge) {
                inViewThicknessEdge = (winfo.geometry().right() == view->absoluteGeometry().left() - 1);
            }

            if (view->formFactor() == Plasma::Types::Horizontal) {
                int yCenter = view->absoluteGeometry().center().y();

                QPoint leftChecker(winfo.geometry().left(), yCenter);
                QPoint rightChecker(winfo.geometry().right(), yCenter);

                bool fulloverlap = (winfo.geometry().left()<=view->absoluteGeometry().left()) && (winfo.geometry().right()>=view->absoluteGeometry().right());

                inViewLengthBoundaries = fulloverlap || view->absoluteGeometry().contains(leftChecker) || view->absoluteGeometry().contains(rightChecker);
            } else if (view->formFactor() == Plasma::Types::Vertical) {
                int xCenter = view->absoluteGeometry().center().x();

                QPoint topChecker(xCenter, winfo.geometry().top());
                QPoint bottomChecker(xCenter, winfo.geometry().bottom());

                bool fulloverlap = (winfo.geometry().top()<=view->absoluteGeometry().top()) && (winfo.geometry().bottom()>=view->absoluteGeometry().bottom());

                inViewLengthBoundaries = fulloverlap || view->absoluteGeometry().contains(topChecker) || view->absoluteGeometry().contains(bottomChecker);
            }
        }

        return (inViewThicknessEdge && inViewLengthBoundaries);
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


void Windows::updateAvailableScreenGeometries()
{
    for (const auto view : m_views.keys()) {
        if (m_views[view]->enabled()) {
            int currentScrId = view->positioner()->currentScreenId();
            QRect tempAvailableScreenGeometry = m_wm->corona()->availableScreenRectWithCriteria(currentScrId, {Types::AlwaysVisible}, {});

            if (tempAvailableScreenGeometry != m_views[view]->availableScreenGeometry()) {
                m_views[view]->setAvailableScreenGeometry(tempAvailableScreenGeometry);

                updateHints(view);
            }
        }
    }
}

void Windows::setPlasmaDesktop(WindowId wid)
{
    if (!m_windows.contains(wid)) {
        return;
    }

    if (!m_windows[wid].isPlasmaDesktop()) {
        m_windows[wid].setIsPlasmaDesktop(true);
        qDebug() << " plasmashell updated...";
        updateAllHints();
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
                    bool topTouch = verView->isTouchingTopViewAndIsBusy() && horView->location() == Plasma::Types::TopEdge;
                    bool bottomTouch = verView->isTouchingBottomViewAndIsBusy() && horView->location() == Plasma::Types::BottomEdge;

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
    bool foundTouchInCurScreen{false};
    bool foundMaximizedInCurScreen{false};

    bool foundActiveGroupTouchInCurScreen{false};

    //! the notification window is not sending a remove signal and creates windows of geometry (0x0 0,0),
    //! maybe a garbage collector here is a good idea!!!
    bool existsFaultyWindow{false};

    WindowId maxWinId;
    WindowId activeWinId;
    WindowId touchWinId;
    WindowId activeTouchWinId;

    //! First Pass
    for (const auto &winfo : m_windows) {
        if (!existsFaultyWindow && (winfo.wid()<=0 || winfo.geometry() == QRect(0, 0, 0, 0))) {
            existsFaultyWindow = true;
        }

        if (winfo.isPlasmaDesktop() || !m_wm->inCurrentDesktopActivity(winfo) || m_wm->isRegisteredPlasmaPanel(winfo.wid())) {
            continue;
        }

        if (isActive(winfo)) {
            foundActive = true;
        }

        if (isActiveInViewScreen(view, winfo)) {
            foundActiveInCurScreen = true;
            activeWinId = winfo.wid();
        }

        if (isTouchingViewEdge(view, winfo) || isTouchingView(view, winfo)) {
            if (winfo.isActive()) {
                //qDebug() << " ACTIVE-TOUCH :: " << winfo.wid() << " _ " << winfo.appName() << " _ " << winfo.geometry() << " _ " << winfo.display();
                foundActiveTouchInCurScreen = true;
                activeTouchWinId = winfo.wid();

                if (isMaximizedInViewScreen(view, winfo)) {
                    //! active maximized windows have higher priority than the rest maximized windows
                    foundMaximizedInCurScreen = true;
                    maxWinId = winfo.wid();
                }
            } else {
                //qDebug() << " TOUCH :: " << winfo.wid() << " _ " << winfo.appName() << " _ " << winfo.geometry() << " _ " << winfo.display();
                foundTouchInCurScreen = true;
                touchWinId = winfo.wid();
            }

            if (!foundMaximizedInCurScreen && isMaximizedInViewScreen(view, winfo)) {
                foundMaximizedInCurScreen = true;
                maxWinId = winfo.wid();
            }
        }

        //qDebug() << "window geometry ::: " << winfo.geometry();
    }

    if (existsFaultyWindow) {
        cleanupFaultyWindows();
    }

    //! PASS 2
    if (foundActiveInCurScreen && !foundActiveTouchInCurScreen) {
        //! Second Pass to track also Child windows if needed

        //qDebug() << "Windows Array...";
        //for (const auto &winfo : m_windows) {
        //    qDebug() << " - " << winfo.wid() << " - " << winfo.isValid() << " - " << winfo.display() << " - " << winfo.geometry() << " parent : " << winfo.parentId();
        //}
        //qDebug() << " - - - - - ";

        WindowInfoWrap activeInfo = m_windows[activeWinId];
        WindowId mainWindowId = activeInfo.isChildWindow() ? activeInfo.parentId() : activeWinId;

        for (const auto &winfo : m_windows) {
            if (winfo.isPlasmaDesktop() || !m_wm->inCurrentDesktopActivity(winfo) || m_wm->isRegisteredPlasmaPanel(winfo.wid())) {
                continue;
            }

            bool inActiveGroup = (winfo.wid() == mainWindowId || winfo.parentId() == mainWindowId);

            //! consider only windows that belong to active window group meaning the main window
            //! and its children
            if (!inActiveGroup) {
                continue;
            }

            if (isTouchingViewEdge(view, winfo) || isTouchingView(view, winfo)) {
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
    setActiveWindowMaximized(view, (maxWinId.toInt()>0 && (maxWinId == activeTouchWinId)));
    setExistsWindowMaximized(view, foundMaximizedInCurScreen);
    setExistsWindowTouching(view, (foundTouchInCurScreen || foundActiveTouchInCurScreen || foundActiveGroupTouchInCurScreen));

    //! update color schemes for active and touching windows
    setActiveWindowScheme(view, (foundActiveInCurScreen ? m_wm->schemesTracker()->schemeForWindow(activeWinId) : nullptr));

    if (foundActiveTouchInCurScreen) {
        setTouchingWindowScheme(view, m_wm->schemesTracker()->schemeForWindow(activeTouchWinId));
    } else if (foundMaximizedInCurScreen) {
        setTouchingWindowScheme(view, m_wm->schemesTracker()->schemeForWindow(maxWinId));
    } else if (foundTouchInCurScreen) {
        setTouchingWindowScheme(view, m_wm->schemesTracker()->schemeForWindow(touchWinId));
    } else {
        setTouchingWindowScheme(view, nullptr);
    }

    //! update LastActiveWindow
    if (foundActiveInCurScreen) {
        m_views[view]->setActiveWindow(activeWinId);
    }

    //! Debug
    //qDebug() << " -- TRACKING REPORT (SCREEN)--";
    //qDebug() << "TRACKING | SCREEN: " << view->positioner()->currentScreenId() << " , EDGE:" << view->location() << " , ENABLED:" << enabled(view);
    //qDebug() << "TRACKING | activeWindowTouching: " << foundActiveTouchInCurScreen << " ,activeWindowMaximized: " << activeWindowMaximized(view);
    //qDebug() << "TRACKING | existsWindowActive: " << foundActiveInCurScreen << " , existsWindowMaximized:" << existsWindowMaximized(view)
    //         << " , existsWindowTouching:"<<existsWindowTouching(view);
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
        if (!existsFaultyWindow && (winfo.wid()<=0 || winfo.geometry() == QRect(0, 0, 0, 0))) {
            existsFaultyWindow = true;
        }

        if (winfo.isPlasmaDesktop() || !m_wm->inCurrentDesktopActivity(winfo) || m_wm->isRegisteredPlasmaPanel(winfo.wid())) {
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
