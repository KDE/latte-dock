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

#ifndef PRIMARYCONFIGVIEW_H
#define PRIMARYCONFIGVIEW_H

// local
#include "../../plasma/quick/configview.h"
#include "../../wm/windowinfowrap.h"
#include "../../../liblatte2/types.h"

//Qt
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QWindow>

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
class SecondaryConfigView;
}
}

namespace Latte {
namespace ViewPart {

class PrimaryConfigView : public PlasmaQuick::ConfigView
{
    Q_OBJECT
    //! used when the secondary config window can not be shown
    Q_PROPERTY(bool showInlineProperties READ showInlineProperties NOTIFY showInlinePropertiesChanged)

    Q_PROPERTY(int complexity READ complexity WRITE setComplexity NOTIFY complexityChanged)

    Q_PROPERTY(QRect availableScreenGeometry READ availableScreenGeometry NOTIFY availableScreenGeometryChanged)

    Q_PROPERTY(Plasma::FrameSvg::EnabledBorders enabledBorders READ enabledBorders NOTIFY enabledBordersChanged)

public:
    enum ConfigViewType
    {
        PrimaryConfig = 0,
        SecondaryConfig
    };

    PrimaryConfigView(Plasma::Containment *containment, Latte::View *view, QWindow *parent = nullptr);
    ~PrimaryConfigView() override;

    void init() override;
    void requestActivate();

    Qt::WindowFlags wFlags() const;

    bool showInlineProperties() const;
    bool sticker() const;

    int complexity() const;
    void setComplexity(int complexity);

    QRect availableScreenGeometry() const;
    QRect geometryWhenVisible() const;

    Plasma::FrameSvg::EnabledBorders enabledBorders() const;

    QQuickView *secondaryWindow();

public slots:
    Q_INVOKABLE void hideConfigWindow();
    Q_INVOKABLE void setSticker(bool blockFocusLost);
    Q_INVOKABLE void syncGeometry();
    Q_INVOKABLE void updateLaunchersForGroup(int groupInt);
    Q_INVOKABLE void updateEffects();

signals:
    void availableScreenGeometryChanged();
    void complexityChanged();
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
    void updateAvailableScreenGeometry(View *origin = nullptr);
    void updateEnabledBorders();
    void updateShowInlineProperties();

    void createSecondaryWindow();
    void deleteSecondaryWindow();

    void setShowInlineProperties(bool show);

    void loadConfig();
    void saveConfig();

private:
    void setupWaylandIntegration();

    bool m_blockFocusLost{false};
    bool m_blockFocusLostOnStartup{true};
    bool m_originalByPassWM{false};
    bool m_inReverse{false};    //! it is used by the borders
    bool m_showInlineProperties{false};

    Latte::Types::Visibility m_originalMode{Latte::Types::DodgeActive};
    Latte::Types::SettingsComplexity m_complexity{Latte::Types::BasicSettings};

    QRect m_availableScreenGeometry;
    QRect m_geometryWhenVisible;

    QPointer<Latte::View> m_latteView;
    QPointer<SecondaryConfigView> m_secConfigView;
    QTimer m_screenSyncTimer;
    QTimer m_thicknessSyncTimer;
    QList<QMetaObject::Connection> connections;

    Plasma::FrameSvg::EnabledBorders m_enabledBorders{Plasma::FrameSvg::AllBorders};

    Latte::Corona *m_corona{nullptr};
    Latte::WindowSystem::WindowId m_waylandWindowId;
    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};
};

}
}
#endif //PRIMARYCONFIGVIEW_H

