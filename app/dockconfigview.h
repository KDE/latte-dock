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

#ifndef NOWDOCKCONFIGVIEW_H
#define NOWDOCKCONFIGVIEW_H

#include "plasmaquick/configview.h"
#include "../liblattedock/dock.h"

#include <plasma/package.h>

#include <QObject>
#include <QWindow>
#include <QPointer>
#include <QTimer>

namespace Plasma {
class Applet;
class Containment;
class Types;
}

namespace Latte {

class DockView;

class DockConfigView : public PlasmaQuick::ConfigView {
    Q_OBJECT
    Q_PROPERTY(bool autostart READ autostart WRITE setAutostart NOTIFY autostartChanged)
    Q_PROPERTY(bool raiseDocksTemporary READ raiseDocksTemporary WRITE setRaiseDocksTemporary NOTIFY raiseDocksTemporaryChanged)

public:
    DockConfigView(Plasma::Containment *containment, DockView *dockView, QWindow *parent = nullptr);
    ~DockConfigView() override;

    void init() override;
    Qt::WindowFlags wFlags() const;

    bool autostart() const;
    void setAutostart(bool state);

    bool raiseDocksTemporary() const;
    void setRaiseDocksTemporary(bool state);

public slots:
    Q_INVOKABLE void addPanelSpacer();
    Q_INVOKABLE void setSticker(bool blockFocusLost);
    Q_INVOKABLE void syncGeometry();

signals:
    void autostartChanged();
    void raiseDocksTemporaryChanged();
    void showSignal();

protected:
    void showEvent(QShowEvent *ev) override;
    void hideEvent(QHideEvent *ev) override;
    void focusOutEvent(QFocusEvent *ev) override;

    void syncSlideEffect();

private slots:
    void immutabilityChanged(Plasma::Types::ImmutabilityType type);

signals:
    void aboutApplication();

private:
    bool m_blockFocusLost;

    QPointer<DockView> m_dockView;
    QTimer m_screenSyncTimer;

    QList<QMetaObject::Connection> connections;

    Dock::Visibility m_previousMode{Dock::None};
};

}
#endif //DOCKCONFIGVIEW_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
