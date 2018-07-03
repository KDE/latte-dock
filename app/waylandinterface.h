/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef WAYLANDINTERFACE_H
#define WAYLANDINTERFACE_H

#include "abstractwindowinterface.h"
#include "windowinfowrap.h"

#include <QObject>
#include <QMap>

#include <KWindowInfo>
#include <KWindowEffects>
#include <QSignalMapper>

#include <KWayland/Client/registry.h>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/plasmawindowmanagement.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>

namespace Latte {

class DockCorona;

namespace Private {
/**
 * @brief this class is use for create the struts inside wayland
 */
class GhostWindow;
}

class WaylandInterface : public AbstractWindowInterface
{
    Q_OBJECT

public:
    explicit WaylandInterface(QObject *parent = nullptr);
    ~WaylandInterface() override;

    void setDockExtraFlags(QWindow &view) override;
    void setDockStruts(QWindow &view, const QRect &rect
                       , Plasma::Types::Location location) override;
    void setWindowOnActivities(QWindow &view, const QStringList &activities) override;

    void removeDockStruts(QWindow &view) const override;

    WindowId activeWindow() const override;
    WindowInfoWrap requestInfo(WindowId wid) const override;
    WindowInfoWrap requestInfoActive() const override;
    bool isOnCurrentDesktop(WindowId wid) const override;
    bool isOnCurrentActivity(WindowId wid) const override;
    const std::list<WindowId> &windows() const override;

    void setKeepAbove(const QDialog &dialog, bool above = true) const override;
    void skipTaskBar(const QDialog &dialog) const override;
    void slideWindow(QWindow &view, Slide location) const override;
    void enableBlurBehind(QWindow &view) const override;

    void setEdgeStateFor(QWindow *view, bool active) const override;

    void initWindowManagement(KWayland::Client::PlasmaWindowManagement *windowManagement);

private:
    void init();
    inline bool isValidWindow(const KWayland::Client::PlasmaWindow *w) const;
    void windowCreatedProxy(KWayland::Client::PlasmaWindow *w);
    KWayland::Client::PlasmaShell *waylandDockCoronaInterface() const;

    QSignalMapper *mapper{nullptr};

    friend class Private::GhostWindow;
    mutable QMap<WindowId, Private::GhostWindow *> m_ghostWindows;

    KWayland::Client::PlasmaWindowManagement *m_windowManagement{nullptr};

    DockCorona *m_corona{nullptr};
};


}

#endif // WAYLANDINTERFACE_H


