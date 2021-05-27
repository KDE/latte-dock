/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WIDGETEXPLORERVIEW_H
#define WIDGETEXPLORERVIEW_H

// local
#include "subconfigview.h"

//Qt
#include <QObject>
#include <QQuickView>
#include <QPointer>
#include <QTimer>

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

class WidgetExplorerView : public SubConfigView
{
    Q_OBJECT
    Q_PROPERTY(bool hideOnWindowDeactivate READ hideOnWindowDeactivate WRITE setHideOnWindowDeactivate NOTIFY hideOnWindowDeactivateChanged)

public:
    WidgetExplorerView(Latte::View *view);

    bool hideOnWindowDeactivate() const;
    void setHideOnWindowDeactivate(bool hide);

    QRect geometryWhenVisible() const;

public slots:
    Q_INVOKABLE void hideConfigWindow();
    Q_INVOKABLE void syncGeometry() override;
    Q_INVOKABLE void updateEffects();

signals:
    void hideOnWindowDeactivateChanged();
    void showSignal();

protected:
    void showEvent(QShowEvent *ev) override;
    void syncSlideEffect() override;
    void focusOutEvent(QFocusEvent *ev) override;

    void init() override;
    void initParentView(Latte::View *view) override;
    void updateEnabledBorders() override;

    Qt::WindowFlags wFlags() const override;

private:
    QRect availableScreenGeometry() const;

private:
    bool m_hideOnWindowDeactivate{true};
    QRect m_geometryWhenVisible;

    //only for the mask on disabled compositing, not to actually paint
    Plasma::FrameSvg *m_background{nullptr};
};

}
}
#endif //WIDGETEXPLORERVIEW_H

