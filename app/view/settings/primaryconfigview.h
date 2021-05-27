/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

namespace Config{
class IndicatorUiManager;
}
}
}

namespace Latte {
namespace ViewPart {

class PrimaryConfigView : public SubConfigView
{
    Q_OBJECT
    //! used when the secondary config window can not be shown
    Q_PROPERTY(bool showInlineProperties READ showInlineProperties NOTIFY showInlinePropertiesChanged)
    Q_PROPERTY(bool isReady READ isReady NOTIFY isReadyChanged)

    Q_PROPERTY(int x READ x NOTIFY xChanged)
    Q_PROPERTY(int y READ y NOTIFY yChanged)

    Q_PROPERTY(QRect availableScreenGeometry READ availableScreenGeometry NOTIFY availableScreenGeometryChanged)

    Q_PROPERTY(Latte::ViewPart::Config::IndicatorUiManager *indicatorUiManager READ indicatorUiManager NOTIFY indicatorUiManagerChanged)

public:
    enum ConfigViewType
    {
        PrimaryConfig = 0,
        SecondaryConfig
    };

    PrimaryConfigView(Latte::View *view);
    ~PrimaryConfigView() override;

    bool hasFocus() const;

    bool isReady() const;

    bool showInlineProperties() const;
    bool sticker() const;

    QRect availableScreenGeometry() const;
    QRect geometryWhenVisible() const;

    Config::IndicatorUiManager *indicatorUiManager();

    void setParentView(Latte::View *view, const bool &immediate = false) override;
    void setOnActivities(QStringList activities);

    void showConfigWindow();

    void requestActivate() override;

public slots:
    Q_INVOKABLE void syncGeometry() override;
    Q_INVOKABLE void hideConfigWindow();
    Q_INVOKABLE void setSticker(bool blockFocusLost);    
    Q_INVOKABLE void updateEffects();

signals:
    void availableScreenGeometryChanged();
    void indicatorUiManagerChanged();
    void isReadyChanged();
    void raiseDocksTemporaryChanged();
    void showInlinePropertiesChanged();
    void showSignal();
    void xChanged();
    void yChanged();

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


private:
    void setIsReady(bool ready);
    void instantUpdateAvailableScreenGeometry();

    bool inAdvancedMode() const;

private:
    bool m_blockFocusLost{false};
    bool m_blockFocusLostOnStartup{true};
    bool m_originalByPassWM{false};
    bool m_inReverse{false};    //! it is used by the borders
    bool m_isReady{false};
    bool m_showInlineProperties{false};

    Latte::Types::Visibility m_originalMode{Latte::Types::DodgeActive};

    QTimer m_availableScreemGeometryTimer;

    QRect m_availableScreenGeometry;
    QRect m_geometryWhenVisible;

    QPointer<SecondaryConfigView> m_secConfigView;
    QPointer<CanvasConfigView> m_canvasConfigView;

    Config::IndicatorUiManager *m_indicatorUiManager{nullptr};

    //only for the mask on disabled compositing, not to actually paint
    Plasma::FrameSvg *m_background{nullptr};
};

}
}
#endif //PRIMARYCONFIGVIEW_H

