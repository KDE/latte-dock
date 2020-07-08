/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

public slots:
    Q_INVOKABLE void syncGeometry() override;
    Q_INVOKABLE void hideConfigWindow();

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

