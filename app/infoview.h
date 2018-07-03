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

#ifndef INFOVIEW_H
#define INFOVIEW_H

#include "dockcorona.h"

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
    InfoView(DockCorona *corona, QString message, QScreen *screen = qGuiApp->primaryScreen(), QWindow *parent = nullptr);
    ~InfoView() override;

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

private:
    void setupWaylandIntegration();

private:
    QString m_message;

    QScreen *m_screen{nullptr};

    Plasma::FrameSvg::EnabledBorders m_borders{Plasma::FrameSvg::TopBorder | Plasma::FrameSvg::BottomBorder};

    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};

    DockCorona *m_corona{nullptr};
};

}
#endif //DOCKCONFIGVIEW_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
