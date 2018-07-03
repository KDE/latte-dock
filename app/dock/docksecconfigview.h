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

#ifndef DOCKSECCONFIGVIEW_H
#define DOCKSECCONFIGVIEW_H

#include "../../liblattedock/dock.h"

#include <plasma/package.h>
#include <Plasma/FrameSvg>

#include <QObject>
#include <QQuickView>
#include <QPointer>
#include <QTimer>

namespace Plasma {
class Applet;
class Containment;
class FrameSvg;
class Types;
}

namespace KWayland {
namespace Client {
class PlasmaShellSurface;
}
}

namespace Latte {

class DockCorona;
class DockView;

class DockSecConfigView : public QQuickView
{
    Q_OBJECT

    Q_PROPERTY(Plasma::FrameSvg::EnabledBorders enabledBorders READ enabledBorders NOTIFY enabledBordersChanged)

public:
    DockSecConfigView(DockView *dockView, QWindow *parent);
    ~DockSecConfigView() override;

    void init();
    Qt::WindowFlags wFlags() const;

    Plasma::FrameSvg::EnabledBorders enabledBorders() const;

public slots:
    Q_INVOKABLE void hideConfigWindow();
    Q_INVOKABLE void syncGeometry();

signals:
    void enabledBordersChanged();
    void showSignal();

protected:
    void showEvent(QShowEvent *ev) override;
    void focusOutEvent(QFocusEvent *ev) override;
    bool event(QEvent *e) override;

    void syncSlideEffect();

private slots:
    void updateEnabledBorders();

private:
    void setupWaylandIntegration();

    QPointer<DockView> m_dockView;
    QPointer<QWindow> m_parent;
    QTimer m_screenSyncTimer;
    QTimer m_thicknessSyncTimer;
    QList<QMetaObject::Connection> connections;

    Plasma::FrameSvg::EnabledBorders m_enabledBorders{Plasma::FrameSvg::AllBorders};

    DockCorona *m_corona{nullptr};
    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};
};

}
#endif //DOCKSECCONFIGVIEW_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
