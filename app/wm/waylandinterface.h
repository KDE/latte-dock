/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-FileCopyrightText: 2026 OpenAI
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WAYLANDINTERFACE_H
#define WAYLANDINTERFACE_H

// local
#include <config-latte.h>
#include "abstractwindowinterface.h"
#include "windowinfowrap.h"

// Qt
#include <QHash>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QSet>

namespace TaskManager {
class VirtualDesktopInfo;
class WindowTasksModel;
}

namespace Latte {
class Corona;
namespace Private {
class GhostWindow;
}
}

namespace Latte {
namespace WindowSystem {

class WaylandInterface : public AbstractWindowInterface
{
    Q_OBJECT

public:
    explicit WaylandInterface(QObject *parent = nullptr);
    ~WaylandInterface() override;

    void initWindowManagement();

    void setViewExtraFlags(QObject *view, bool isPanelWindow = true, Latte::Types::Visibility mode = Latte::Types::WindowsGoBelow) override;
    void setViewStruts(QWindow &view, const QRect &rect, Plasma::Types::Location location) override;
    void setWindowOnActivities(const WindowId &wid, const QStringList &nextactivities) override;

    void removeViewStruts(QWindow &view) override;

    WindowId activeWindow() override;
    WindowInfoWrap requestInfo(WindowId wid) override;
    WindowInfoWrap requestInfoActive() override;

    void skipTaskBar(const QDialog &dialog) override;
    void slideWindow(QWindow &view, Slide location) override;
    void enableBlurBehind(QWindow &view) override;

    void requestActivate(WindowId wid) override;
    void requestClose(WindowId wid) override;
    void requestMoveWindow(WindowId wid, QPoint from) override;
    void requestToggleIsOnAllDesktops(WindowId wid) override;
    void requestToggleKeepAbove(WindowId wid) override;
    void requestToggleMinimized(WindowId wid) override;
    void requestToggleMaximized(WindowId wid) override;
    void setKeepAbove(WindowId wid, bool active) override;
    void setKeepBelow(WindowId wid, bool active) override;

    bool windowCanBeDragged(WindowId wid) override;
    bool windowCanBeMaximized(WindowId wid) override;

    QIcon iconFor(WindowId wid) override;
    WindowId winIdFor(QString appId, QRect geometry) override;
    WindowId winIdFor(QString appId, QString title) override;

    AppData appDataFor(WindowId wid) override;

    void setActiveEdge(QWindow *view, bool active) override;

    void switchToNextVirtualDesktop() override;
    void switchToPreviousVirtualDesktop() override;

    void setFrameExtents(QWindow *view, const QMargins &margins) override;
    void setInputMask(QWindow *window, const QRect &rect) override;

    void registerIgnoredWindow(WindowId wid) override;
    void unregisterIgnoredWindow(WindowId wid) override;

private:
    void init();
    void rebuildWindowIndexCache();
    void refreshModelState(const QSet<WindowId> &changedIds = {});

    QModelIndex indexFor(WindowId wid) const;
    WindowId windowIdForIndex(const QModelIndex &index) const;
    WindowId activeWindowFromModel() const;
    WindowInfoWrap requestInfo(const QModelIndex &index) const;

    bool isAcceptableWindow(const QModelIndex &index) const;
    bool isValidWindow(const QModelIndex &index) const;
    bool isFullScreenWindow(const QModelIndex &index) const;
    bool isPlasmaPanel(const QModelIndex &index) const;
    bool isSidepanel(const QModelIndex &index) const;
    bool appIdMatches(const QModelIndex &index, const QString &expected) const;

    void setCurrentDesktop(const QString &desktop);
    void syncDesktops();

private:
    friend class Private::GhostWindow;
    mutable QHash<WindowId, Private::GhostWindow *> m_ghostWindows;

    QPointer<TaskManager::WindowTasksModel> m_windowTasksModel;
    QPointer<TaskManager::VirtualDesktopInfo> m_virtualDesktopInfo;

    QHash<WindowId, QPersistentModelIndex> m_windowIndexes;
    QSet<WindowId> m_trackedWindows;
    WindowId m_activeWindowId;

    QStringList m_desktops;
    Latte::Corona *m_corona{nullptr};
};

}
}

#endif // WAYLANDINTERFACE_H
