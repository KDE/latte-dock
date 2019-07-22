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

#ifndef WINDOWSYSTEMWINDOWSTRACKER_H
#define WINDOWSYSTEMWINDOWSTRACKER_H

// local
#include "../windowinfowrap.h"

// Qt
#include <QObject>

#include <QHash>
#include <QMap>

namespace Latte {
class View;
namespace Layout {
class GenericLayout;
}
namespace WindowSystem {
class AbstractWindowInterface;
class SchemeColors;
namespace Tracker {
class LastActiveWindow;
class TrackedLayoutInfo;
class TrackedViewInfo;
}
}
}

namespace Latte {
namespace WindowSystem {
namespace Tracker {

class Windows : public QObject {
    Q_OBJECT

public:
    Windows(AbstractWindowInterface *parent);
    ~Windows() override;

    void addView(Latte::View *view);
    void removeView(Latte::View *view);

    //! Views Tracking (current screen specific)
    bool enabled(Latte::View *view);
    void setEnabled(Latte::View *view, const bool enabled);

    bool activeWindowMaximized(Latte::View *view) const;
    bool activeWindowTouching(Latte::View *view) const;
    bool existsWindowActive(Latte::View *view) const;
    bool existsWindowMaximized(Latte::View *view) const;
    bool existsWindowTouching(Latte::View *view) const;
    SchemeColors *activeWindowScheme(Latte::View *view) const;
    SchemeColors *touchingWindowScheme(Latte::View *view) const;
    LastActiveWindow *lastActiveWindow(Latte::View *view);

    //! Layouts Tracking (all screens)
    bool enabled(Latte::Layout::GenericLayout *layout);
    bool activeWindowMaximized(Latte::Layout::GenericLayout *layout) const;
    bool existsWindowActive(Latte::Layout::GenericLayout *layout) const;
    bool existsWindowMaximized(Latte::Layout::GenericLayout *layout) const;
    SchemeColors *activeWindowScheme(Latte::Layout::GenericLayout *layout) const;
    LastActiveWindow *lastActiveWindow(Latte::Layout::GenericLayout *layout);

    //! Windows management
    bool isValidFor(const WindowId &wid) const;
    QIcon iconFor(const WindowId &wid);
    QString appNameFor(const WindowId &wid);
    WindowInfoWrap infoFor(const WindowId &wid) const;

    void setPlasmaDesktop(WindowId wid);

    AbstractWindowInterface *wm();

signals:
    //! Views
    void enabledChanged(const Latte::View *view);
    void activeWindowMaximizedChanged(const Latte::View *view);
    void activeWindowTouchingChanged(const Latte::View *view);
    void existsWindowActiveChanged(const Latte::View *view);
    void existsWindowMaximizedChanged(const Latte::View *view);
    void existsWindowTouchingChanged(const Latte::View *view);
    void activeWindowSchemeChanged(const Latte::View *view);
    void touchingWindowSchemeChanged(const Latte::View *view);
    void informationAnnounced(const Latte::View *view);

    //! Layouts
    void enabledChangedForLayout(const Latte::Layout::GenericLayout *layout);
    void activeWindowMaximizedChangedForLayout(const Latte::Layout::GenericLayout *layout);
    void existsWindowActiveChangedForLayout(const Latte::Layout::GenericLayout *layout);
    void existsWindowMaximizedChangedForLayout(const Latte::Layout::GenericLayout *layout);
    void activeWindowSchemeChangedForLayout(const Latte::Layout::GenericLayout *layout);
    void informationAnnouncedForLayout(const Latte::Layout::GenericLayout *layout);

    //! overloading WM signals in order to update first m_windows and afterwards
    //! inform consumers for window changes
    void activeWindowChanged(const WindowId &wid);
    void windowChanged(const WindowId &wid);
    void windowRemoved(const WindowId &wid);

private slots:
    void updateAvailableScreenGeometries();

    void addRelevantLayout(Latte::View *view);

    void updateRelevantLayouts();

private:
    void init();
    void initLayoutHints(Latte::Layout::GenericLayout *layout);
    void initViewHints(Latte::View *view);
    void cleanupFaultyWindows();

    void updateAllHints();

    //! Views
    void updateHints(Latte::View *view);
    void updateHints(Latte::Layout::GenericLayout *layout);

    void setActiveWindowMaximized(Latte::View *view, bool activeMaximized);
    void setActiveWindowTouching(Latte::View *view, bool activeTouching);
    void setExistsWindowActive(Latte::View *view, bool windowActive);
    void setExistsWindowMaximized(Latte::View *view, bool windowMaximized);
    void setExistsWindowTouching(Latte::View *view, bool windowTouching);
    void setActiveWindowScheme(Latte::View *view, WindowSystem::SchemeColors *scheme);
    void setTouchingWindowScheme(Latte::View *view, WindowSystem::SchemeColors *scheme);

    //! Layouts
    void setActiveWindowMaximized(Latte::Layout::GenericLayout *layout, bool activeMaximized);
    void setExistsWindowActive(Latte::Layout::GenericLayout *layout, bool windowActive);
    void setExistsWindowMaximized(Latte::Layout::GenericLayout *layout, bool windowMaximized);
    void setActiveWindowScheme(Latte::Layout::GenericLayout *layout, WindowSystem::SchemeColors *scheme);

    //! Windows
    bool inCurrentDesktopActivity(const WindowInfoWrap &winfo);
    bool intersects(Latte::View *view, const WindowInfoWrap &winfo);
    bool isActive(const WindowInfoWrap &winfo);
    bool isActiveInViewScreen(Latte::View *view, const WindowInfoWrap &winfo);
    bool isMaximizedInViewScreen(Latte::View *view, const WindowInfoWrap &winfo);
    bool isTouchingView(Latte::View *view, const WindowSystem::WindowInfoWrap &winfo);
    bool isTouchingViewEdge(Latte::View *view, const WindowInfoWrap &winfo);

private:
    AbstractWindowInterface *m_wm;
    QHash<Latte::View *, TrackedViewInfo *> m_views;
    QHash<Latte::Layout::GenericLayout *, TrackedLayoutInfo *> m_layouts;

    QMap<WindowId, WindowInfoWrap> m_windows;
};

}
}
}

#endif
