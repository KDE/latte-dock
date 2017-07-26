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

namespace KWayland {
namespace Client {
class PlasmaShellSurface;
}
}

namespace Latte {

class InfoView : public QQuickView {
    Q_OBJECT

public:
    InfoView(DockCorona *corona, QString message, QScreen *screen = qGuiApp->primaryScreen(), QWindow *parent = nullptr);
    ~InfoView() override;

    void init();
    Qt::WindowFlags wFlags() const;

public slots:
    Q_INVOKABLE void syncGeometry();

protected:
    void showEvent(QShowEvent *ev) override;
    bool event(QEvent *e) override;

private:
    void setupWaylandIntegration();

private:
    QString m_message;

    QScreen *m_screen;
    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};

    DockCorona *m_corona;
};

}
#endif //DOCKCONFIGVIEW_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
