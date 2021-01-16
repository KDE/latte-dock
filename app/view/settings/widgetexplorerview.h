/*
*  Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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

