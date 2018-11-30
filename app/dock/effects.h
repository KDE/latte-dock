/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef EFFECTS_H
#define EFFECTS_H

#include <QObject>
#include <QPointer>
#include <QRect>

#include <Plasma/FrameSvg>
#include <Plasma/Theme>

namespace Latte {
class DockView;
}

namespace Latte {
namespace View {

class Effects: public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool drawShadows READ drawShadows WRITE setDrawShadows NOTIFY drawShadowsChanged)
    Q_PROPERTY(bool drawEffects READ drawEffects WRITE setDrawEffects NOTIFY drawEffectsChanged)

    Q_PROPERTY(QRect effectsArea READ effectsArea WRITE setEffectsArea NOTIFY effectsAreaChanged)
    Q_PROPERTY(QRect maskArea READ maskArea WRITE setMaskArea NOTIFY maskAreaChanged)

    Q_PROPERTY(Plasma::FrameSvg::EnabledBorders enabledBorders READ enabledBorders NOTIFY enabledBordersChanged)

public:
    Effects(DockView *parent);
    virtual ~Effects();

    bool drawShadows() const;
    void setDrawShadows(bool draw);

    bool drawEffects() const;
    void setDrawEffects(bool draw);

    bool forceDrawCenteredBorders() const;
    void setForceDrawCenteredBorders(bool draw);

    QRect maskArea() const;
    void setMaskArea(QRect area);

    QRect effectsArea() const;
    void setEffectsArea(QRect area);

    Plasma::FrameSvg::EnabledBorders enabledBorders() const;

public slots:
    void updateEffects();
    void updateEnabledBorders();

signals:
    void drawShadowsChanged();
    void drawEffectsChanged();
    void effectsAreaChanged();
    void enabledBordersChanged();
    void maskAreaChanged();

private slots:
    void init();

private:
    bool m_drawShadows{true};
    bool m_drawEffects{false};
    bool m_forceDrawCenteredBorders{false};

    QRect m_effectsArea;
    QRect m_maskArea;

    QPointer<Latte::DockView> m_view;

    Plasma::Theme m_theme;
    //only for the mask on disabled compositing, not to actually paint
    Plasma::FrameSvg *m_background{nullptr};

    //only for the mask, not to actually paint
    Plasma::FrameSvg::EnabledBorders m_enabledBorders{Plasma::FrameSvg::AllBorders};
};

}
}

#endif
