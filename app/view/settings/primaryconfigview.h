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
#include <coretypes.h>
#include "subconfigview.h"
#include "../../wm/windowinfowrap.h"

//Qt
#include <QObject>
#include <QPointer>
#include <QQuickView>
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
class CanvasConfigView;
class SecondaryConfigView;
}
}

namespace Latte {
namespace ViewPart {

class PrimaryConfigView : public SubConfigView
{
    Q_OBJECT
    //! used when the secondary config window can not be shown
    Q_PROPERTY(bool showInlineProperties READ showInlineProperties NOTIFY showInlinePropertiesChanged)
    Q_PROPERTY(bool inAdvancedMode READ inAdvancedMode WRITE setInAdvancedMode NOTIFY inAdvancedModeChanged)
    Q_PROPERTY(bool isReady READ isReady NOTIFY isReadyChanged)

    Q_PROPERTY(QRect availableScreenGeometry READ availableScreenGeometry NOTIFY availableScreenGeometryChanged)

public:
    enum ConfigViewType
    {
        PrimaryConfig = 0,
        SecondaryConfig
    };

    PrimaryConfigView(Latte::View *view);
    ~PrimaryConfigView() override;

    bool inAdvancedMode() const;
    void setInAdvancedMode(bool advanced);

    bool isReady() const;

    bool showInlineProperties() const;
    bool sticker() const;

    QRect availableScreenGeometry() const;
    QRect geometryWhenVisible() const;

    void setParentView(Latte::View *view) override;
    void setOnActivities(QStringList activities);

    void showPrimaryWindow();
    void hidePrimaryWindow();

    void requestActivate() override;

public slots:
    Q_INVOKABLE void syncGeometry() override;
    Q_INVOKABLE void hideConfigWindow();
    Q_INVOKABLE void setSticker(bool blockFocusLost);    
    Q_INVOKABLE void updateEffects();

signals:
    void availableScreenGeometryChanged();
    void inAdvancedModeChanged();
    void isReadyChanged();
    void raiseDocksTemporaryChanged();
    void showInlinePropertiesChanged();
    void showSignal();

protected:
    void showEvent(QShowEvent *ev) override;
    void hideEvent(QHideEvent *ev) override;
    void focusOutEvent(QFocusEvent *ev) override;

    void init() override;
    void initParentView(Latte::View *view) override;
    void updateEnabledBorders() override;

private slots:
    void immutabilityChanged(Plasma::Types::ImmutabilityType type);
    void updateAvailableScreenGeometry(View *origin = nullptr);
    void updateShowInlineProperties();

    void showSecondaryWindow();
    void hideSecondaryWindow();

    void showCanvasWindow();
    void hideCanvasWindow();

    void setShowInlineProperties(bool show);

    void loadConfig();
    void saveConfig();

private:
    void setIsReady(bool ready);

private:
    bool m_blockFocusLost{false};
    bool m_blockFocusLostOnStartup{true};
    bool m_originalByPassWM{false};
    bool m_inAdvancedMode{false};
    bool m_inReverse{false};    //! it is used by the borders
    bool m_isReady{false};
    bool m_showInlineProperties{false};

    Latte::Types::Visibility m_originalMode{Latte::Types::DodgeActive};

    QRect m_availableScreenGeometry;
    QRect m_geometryWhenVisible;

    QPointer<SecondaryConfigView> m_secConfigView;
    QPointer<CanvasConfigView> m_canvasConfigView;

    //only for the mask on disabled compositing, not to actually paint
    Plasma::FrameSvg *m_background{nullptr};
};

}
}
#endif //PRIMARYCONFIGVIEW_H

