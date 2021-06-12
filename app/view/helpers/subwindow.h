/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWSUBWINDOW_H
#define VIEWSUBWINDOW_H

// local
#include "../../lattecorona.h"
#include "../../wm/windowinfowrap.h"

// Qt
#include <QObject>
#include <QQuickView>
#include <QTimer>

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

//! What is the importance of this class?
//!
//! This window is responsible to provide a common window base for ViewPart::Helpers

class SubWindow : public QQuickView
{
    Q_OBJECT

public:
    SubWindow(Latte::View *view, QString debugType);
    ~SubWindow() override;

    int location();
    int thickness() const;

    QString validTitle() const;

    void hideWithMask();
    void showWithMask();

    Latte::View *parentView();
    Latte::WindowSystem::WindowId trackedWindowId();

    KWayland::Client::PlasmaShellSurface *surface();

signals:
    void forcedShown(); //[workaround] forced shown to avoid a KWin issue that hides windows when activities are stopped
    void calculatedGeometryChanged();

protected:
    bool event(QEvent *ev) override;

    virtual QString validTitlePrefix() const;

    //! it is used to update m_calculatedGeometry correctly
    virtual void updateGeometry() = 0;

private slots:
    void startGeometryTimer();
    void fixGeometry();
    void updateWaylandId();

private:
    void setupWaylandIntegration();

protected:
    bool m_debugMode{false};
    bool m_inDelete{false};

    int m_thickness{2};

    QString m_debugType;

    QRect m_calculatedGeometry;

    //! [workaround] colors in order to help masking to apply immediately
    //! for some reason when the window in with no content the mask is not
    //! update immediately
    QColor m_hideColor;
    QColor m_showColor;

    QTimer m_fixGeometryTimer;

    //! HACK: Timers in order to handle KWin faulty
    //! behavior that hides Views when closing Activities
    //! with no actual reason
    QTimer m_visibleHackTimer1;
    QTimer m_visibleHackTimer2;
    //! Connections for the KWin visibility hack
    QList<QMetaObject::Connection> connectionsHack;

    Latte::View *m_latteView{nullptr};

    QPointer<Latte::Corona> m_corona;

    Latte::WindowSystem::WindowId m_trackedWindowId;
    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};
};

}
}
#endif
