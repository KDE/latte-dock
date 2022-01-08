/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WAYLANDINTERFACE_H
#define WAYLANDINTERFACE_H

// local
#include <config-latte.h>
#include "abstractwindowinterface.h"
#include "windowinfowrap.h"

// Qt
#include <QMap>
#include <QObject>

// KDE
#include <KWayland/Client/registry.h>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/plasmawindowmanagement.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>
#include <KWindowInfo>
#include <KWindowEffects>


namespace Latte {
class Corona;
namespace Private {
//! this class is used to create the struts inside wayland
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

    void setViewExtraFlags(QObject *view, bool isPanelWindow = true, Latte::Types::Visibility mode = Latte::Types::WindowsGoBelow) override;
    void setViewStruts(QWindow &view, const QRect &rect
                       , Plasma::Types::Location location) override;
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

    void setActiveEdge(QWindow *view, bool active)  override;

    void switchToNextVirtualDesktop() override;
    void switchToPreviousVirtualDesktop() override;

    void setFrameExtents(QWindow *view, const QMargins &margins) override;
    void setInputMask(QWindow *window, const QRect &rect) override;

    void registerIgnoredWindow(WindowId wid) override;
    void unregisterIgnoredWindow(WindowId wid) override;

    void initWindowManagement(KWayland::Client::PlasmaWindowManagement *windowManagement);

    //! VirtualDesktopsSupport
    void initVirtualDesktopManagement(KWayland::Client::PlasmaVirtualDesktopManagement *virtualDesktopManagement);


private slots:
    void updateWindow();
    void windowUnmapped();

private:
    void init();
    bool isAcceptableWindow(const KWayland::Client::PlasmaWindow *w);
    bool isValidWindow(const KWayland::Client::PlasmaWindow *w);
    bool isFullScreenWindow(const KWayland::Client::PlasmaWindow *w) const;
    bool isPlasmaPanel(const KWayland::Client::PlasmaWindow *w) const;
    bool isSidepanel(const KWayland::Client::PlasmaWindow *w) const;
    void windowCreatedProxy(KWayland::Client::PlasmaWindow *w);
    void trackWindow(KWayland::Client::PlasmaWindow *w);
    void untrackWindow(KWayland::Client::PlasmaWindow *w);

    KWayland::Client::PlasmaWindow *windowFor(WindowId wid);
    KWayland::Client::PlasmaShell *waylandCoronaInterface() const;

    //! VirtualDesktopsSupport
    void setCurrentDesktop(QString desktop);
    void addDesktop(const QString &id, quint32 position);


private:
    friend class Private::GhostWindow;
    mutable QMap<WindowId, Private::GhostWindow *> m_ghostWindows;

    KWayland::Client::PlasmaWindowManagement *m_windowManagement{nullptr};

    //! VirtualDesktopsSupport
    KWayland::Client::PlasmaVirtualDesktopManagement *m_virtualDesktopManagement{nullptr};
    QStringList m_desktops;


    Latte::Corona *m_corona{nullptr};
};

}
}

#endif // WAYLANDINTERFACE_H


