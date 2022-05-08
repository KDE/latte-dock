/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INFOVIEW_H
#define INFOVIEW_H

// local
#include "lattecorona.h"
#include "wm/windowinfowrap.h"

// Qt
#include <QObject>
#include <QQuickView>
#include <QScreen>

namespace KWayland {
namespace Client {
class PlasmaShellSurface;
}
}

namespace Latte {

class InfoView : public QQuickView
{
    Q_OBJECT

    Q_PROPERTY(Plasma::FrameSvg::EnabledBorders enabledBorders READ enabledBorders NOTIFY enabledBordersChanged)

public:
    InfoView(Latte::Corona *corona, QString message, QScreen *screen, QWindow *parent = nullptr);
    ~InfoView() override;

    QString validTitle() const;

    Plasma::FrameSvg::EnabledBorders enabledBorders() const;

    void init();
    Qt::WindowFlags wFlags() const;

    void setOnActivities(QStringList activities = {"0"});

public slots:
    Q_INVOKABLE void syncGeometry();

signals:
    void enabledBordersChanged();

protected:
    void showEvent(QShowEvent *ev) override;
    bool event(QEvent *e) override;

private slots:
    void setupWaylandIntegration();
    void updateWaylandId();

private:
    QString m_id;

    QString m_message;

    QScreen *m_screen{nullptr};

    Plasma::FrameSvg::EnabledBorders m_borders{Plasma::FrameSvg::TopBorder | Plasma::FrameSvg::BottomBorder};

    Latte::WindowSystem::WindowId m_trackedWindowId;
    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};

    Latte::Corona *m_corona{nullptr};
};

}
#endif //INFOVIEW_H
