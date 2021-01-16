/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef SUBCONFIGVIEW_H
#define SUBCONFIGVIEW_H

// local
#include <coretypes.h>
#include "../../wm/windowinfowrap.h"

//Qt
#include <QObject>
#include <QPointer>
#include <QQuickView>
#include <QTimer>

// Plasma
#include <Plasma/FrameSvg>

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

class SubConfigView : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(Plasma::FrameSvg::EnabledBorders enabledBorders READ enabledBorders NOTIFY enabledBordersChanged)

public:
    SubConfigView(Latte::View *view, const QString &title, const bool &isNormalWindow = true);
    ~SubConfigView() override;

    virtual void requestActivate();

    QString validTitle() const;

    Plasma::FrameSvg::EnabledBorders enabledBorders() const;

    Latte::Corona *corona() const;
    Latte::View *parentView() const;
    virtual void setParentView(Latte::View *view, const bool &immediate = false);
    virtual void showAfter(int msecs = 0);

    Latte::WindowSystem::WindowId trackedWindowId();
    KWayland::Client::PlasmaShellSurface *surface();

public slots:
    virtual void syncGeometry() = 0;

signals:
    void enabledBordersChanged();

protected:
    virtual void syncSlideEffect();

    virtual void init();
    virtual void initParentView(Latte::View *view);
    virtual void updateEnabledBorders() = 0;

    void showEvent(QShowEvent *ev) override;
    bool event(QEvent *e) override;

    virtual Qt::WindowFlags wFlags() const;

protected:
    bool m_isNormalWindow{true};
    QTimer m_screenSyncTimer;

    QPointer<Latte::View> m_latteView;

    QList<QMetaObject::Connection> connections;
    QList<QMetaObject::Connection> viewconnections;

    Plasma::FrameSvg::EnabledBorders m_enabledBorders{Plasma::FrameSvg::AllBorders};

    Latte::Corona *m_corona{nullptr};
    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};

private slots:
    void updateWaylandId();

private:
    void setupWaylandIntegration();

private:
    QString m_validTitle;

    QTimer m_showTimer;

    Latte::WindowSystem::WindowId m_waylandWindowId;
};

}
}
#endif //SUBCONFIGVIEW_H

