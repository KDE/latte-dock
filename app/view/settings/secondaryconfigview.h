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

#ifndef SECONDARYCONFIGVIEW_H
#define SECONDARYCONFIGVIEW_H

// local
#include "../../../liblatte2/types.h"

//Qt
#include <QObject>
#include <QQuickView>
#include <QPointer>
#include <QTimer>

// Plasma
#include <plasma/package.h>
#include <Plasma/FrameSvg>

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
class Corona;
class View;
}

namespace Latte {
namespace ViewPart {
class PrimaryConfigView;
}
}

namespace Latte {
namespace ViewPart {

class SecondaryConfigView : public QQuickView
{
    Q_OBJECT

    Q_PROPERTY(Plasma::FrameSvg::EnabledBorders enabledBorders READ enabledBorders NOTIFY enabledBordersChanged)

public:
    SecondaryConfigView(Latte::View *view, QWindow *parent);
    ~SecondaryConfigView() override;

    void init();
    void requestActivate();
    Qt::WindowFlags wFlags() const;

    QRect geometryWhenVisible() const;

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

    QRect m_geometryWhenVisible;

    QPointer<Latte::View> m_latteView;
    QPointer<PrimaryConfigView> m_parent;
    QTimer m_screenSyncTimer;
    QTimer m_thicknessSyncTimer;
    QList<QMetaObject::Connection> connections;

    Plasma::FrameSvg::EnabledBorders m_enabledBorders{Plasma::FrameSvg::AllBorders};

    Latte::Corona *m_corona{nullptr};
    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};
};

}
}
#endif //SECONDARYCONFIGVIEW_H

