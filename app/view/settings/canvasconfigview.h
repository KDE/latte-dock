/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CANVASCONFIGVIEW_H
#define CANVASCONFIGVIEW_H

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
class PrimaryConfigView;
}
}

namespace Latte {
namespace ViewPart {

class CanvasConfigView : public SubConfigView
{
    Q_OBJECT

public:
    CanvasConfigView(Latte::View *view, PrimaryConfigView *parent);

    QRect geometryWhenVisible() const;

    void hideConfigWindow();

public slots:
    Q_INVOKABLE void syncGeometry() override;

signals:
    void showSignal();

protected:
    void showEvent(QShowEvent *ev) override;
    void focusOutEvent(QFocusEvent *ev) override;
    bool event(QEvent *ev) override;

    void init() override;
    void initParentView(Latte::View *view) override;
    void updateEnabledBorders() override;

private:
    QRect m_geometryWhenVisible;

    QPointer<PrimaryConfigView> m_parent;
};

}
}
#endif //CANVASSECONDARYCONFIGVIEW_H

