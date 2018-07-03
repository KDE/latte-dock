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

#ifndef DOCKCONFIGVIEW_H
#define DOCKCONFIGVIEW_H

#include "../plasmaquick/configview.h"
#include "../../liblattedock/dock.h"
#include "docksecconfigview.h"

#include <plasma/package.h>
#include <Plasma/FrameSvg>

#include <QObject>
#include <QWindow>
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

class DockConfigView : public PlasmaQuick::ConfigView
{
    Q_OBJECT
    //! used when the secondary config window can not be shown
    Q_PROPERTY(bool showInlineProperties READ showInlineProperties WRITE setShowInlineProperties NOTIFY showInlinePropertiesChanged)

    Q_PROPERTY(Plasma::FrameSvg::EnabledBorders enabledBorders READ enabledBorders NOTIFY enabledBordersChanged)

public:
    enum ConfigViewType
    {
        PrimaryConfig = 0,
        SecondaryConfig
    };

    DockConfigView(Plasma::Containment *containment, DockView *dockView, QWindow *parent = nullptr);
    ~DockConfigView() override;

    void init() override;
    Qt::WindowFlags wFlags() const;

    bool showInlineProperties() const;
    void setShowInlineProperties(bool show);

    bool sticker() const;

    Plasma::FrameSvg::EnabledBorders enabledBorders() const;

    QWindow *secondaryWindow();

public slots:
    Q_INVOKABLE void addPanelSpacer();
    Q_INVOKABLE void createSecondaryWindow();
    Q_INVOKABLE void deleteSecondaryWindow();
    Q_INVOKABLE void hideConfigWindow();
    Q_INVOKABLE void setSticker(bool blockFocusLost);
    Q_INVOKABLE void syncGeometry();
    Q_INVOKABLE void updateLaunchersForGroup(int groupInt);

signals:
    void enabledBordersChanged();
    void raiseDocksTemporaryChanged();
    void showInlinePropertiesChanged();
    void showSignal();

protected:
    void showEvent(QShowEvent *ev) override;
    void hideEvent(QHideEvent *ev) override;
    void focusOutEvent(QFocusEvent *ev) override;
    bool event(QEvent *e) override;

    void syncSlideEffect();

private slots:
    void immutabilityChanged(Plasma::Types::ImmutabilityType type);
    void updateEnabledBorders();

private:
    void setupWaylandIntegration();

    bool m_blockFocusLost{false};
    bool m_blockFocusLostOnStartup{true};
    bool m_inReverse{false};    //! it is used by the borders
    bool m_showInlineProperties{false};

    QPointer<DockView> m_dockView;
    QPointer<DockSecConfigView> m_secConfigView;
    QTimer m_screenSyncTimer;
    QTimer m_thicknessSyncTimer;
    QList<QMetaObject::Connection> connections;

    Plasma::FrameSvg::EnabledBorders m_enabledBorders{Plasma::FrameSvg::AllBorders};

    DockCorona *m_corona{nullptr};
    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};
};

}
#endif //DOCKCONFIGVIEW_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
