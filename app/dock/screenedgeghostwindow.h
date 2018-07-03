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

#ifndef SCREENEDGEGHOSTWINDOW_H
#define SCREENEDGEGHOSTWINDOW_H

#include <QObject>
#include <QQuickView>
#include <QTimer>

namespace KWayland {
namespace Client {
class PlasmaShellSurface;
}
}

namespace Latte {
class DockView;
}

namespace Latte {

//! What is the importance of this class?
//!
//! Plasma is activating the screen edges for the main panel window
//! unfortunately this isnt possible for the Latte case.
//! When a window is hidden at an edge it becomes NOT visible
//! unfortunately that means that all the animations are
//! stopped (Qt behaviour) and that creates confusion to the user after the window
//! reappears because various animations are played (adding-removing tasks/launchers)
//! that arent relevant any more.
//!
//! In order to workaround the above behaviour Latte is using a
//! fake window to communicate with KWin and the MAIN dock window
//! continues to use only mask technique to hide
//!
//! KDE BUGS: https://bugs.kde.org/show_bug.cgi?id=382219
//!           https://bugs.kde.org/show_bug.cgi?id=392464

class ScreenEdgeGhostWindow : public QQuickView
{
    Q_OBJECT

public:
    ScreenEdgeGhostWindow(DockView *view);
    ~ScreenEdgeGhostWindow() override;

    int location();

    void hideWithMask();
    void showWithMask();

    DockView *parentDock();

    KWayland::Client::PlasmaShellSurface *surface();

signals:
    void edgeTriggered();

protected:
    bool event(QEvent *ev) override;

private slots:
    void startGeometryTimer();
    void updateGeometry();
    void fixGeometry();

private:
    void setupWaylandIntegration();

private:
    bool m_inDelete{false};
    QRect m_calculatedGeometry;

    QTimer m_fixGeometryTimer;

    DockView *m_dockView{nullptr};

    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};
};

}

#endif
