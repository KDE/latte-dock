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

#include "effects.h"
#include "dockview.h"
#include "panelshadows_p.h"
#include "../../liblattedock/dock.h"

#include <QRegion>

#include <KWindowEffects>
#include <KWindowSystem>

namespace Latte {
namespace View {

Effects::Effects(Latte::DockView *parent)
    : QObject(parent),
      m_view(parent)
{
    init();
}

Effects::~Effects()
{
}

void Effects::init()
{
    connect(this, &Effects::backgroundOpacityChanged, this, &Effects::updateEffects);
    connect(this, &Effects::drawEffectsChanged, this, &Effects::updateEffects);
    connect(this, &Effects::rectChanged, this, &Effects::updateEffects);

    connect(this, &Effects::drawShadowsChanged, this, [&]() {
        if (m_view->behaveAsPlasmaPanel()) {
            updateEnabledBorders();
        }
    });

    connect(m_view, &Latte::DockView::alignmentChanged, this, &Effects::updateEnabledBorders);
    connect(m_view, &Latte::DockView::behaveAsPlasmaPanelChanged, this, &Effects::updateEffects);

    connect(this, SIGNAL(innerShadowChanged()), m_view->corona(), SIGNAL(availableScreenRectChanged()));
}

bool Effects::animationsBlocked() const
{
    return m_animationsBlocked;
}

void Effects::setAnimationsBlocked(bool blocked)
{
    if (m_animationsBlocked == blocked) {
        return;
    }

    m_animationsBlocked = blocked;
    emit animationsBlockedChanged();
}

bool Effects::colorizerEnabled() const
{
    return m_colorizerEnabled;
}

void Effects::setColorizerEnabled(bool enabled)
{
    if (m_colorizerEnabled == enabled) {
        return;
    }

    m_colorizerEnabled = enabled;
    emit colorizerEnabledChanged();
}


bool Effects::drawShadows() const
{
    return m_drawShadows;
}

void Effects::setDrawShadows(bool draw)
{
    if (m_drawShadows == draw) {
        return;
    }

    m_drawShadows = draw;

    if (m_view->behaveAsPlasmaPanel() && m_drawShadows) {
        PanelShadows::self()->addWindow(m_view, m_enabledBorders);
    } else {
        PanelShadows::self()->removeWindow(m_view);
    }

    emit drawShadowsChanged();
}

bool Effects::drawEffects() const
{
    return m_drawEffects;
}

void Effects::setDrawEffects(bool draw)
{
    if (m_drawEffects == draw) {
        return;
    }

    m_drawEffects = draw;

    emit drawEffectsChanged();
}

bool Effects::forceDrawCenteredBorders() const
{
    return m_forceDrawCenteredBorders;
}

void Effects::setForceDrawCenteredBorders(bool draw)
{
    if (m_forceDrawCenteredBorders == draw) {
        return;
    }

    m_forceDrawCenteredBorders = draw;
}

int Effects::backgroundOpacity() const
{
    return m_backgroundOpacity;
}

void Effects::setBackgroundOpacity(int opacity)
{
    if (m_backgroundOpacity == opacity) {
        return;
    }

    m_backgroundOpacity = opacity;
    emit backgroundOpacityChanged();
}

int Effects::innerShadow() const
{
    return m_innerShadow;
}

void Effects::setInnerShadow(int shadow)
{
    if (m_innerShadow == shadow)
        return;

    m_innerShadow = shadow;

    emit innerShadowChanged();
}

QRect Effects::rect() const
{
    return m_rect;
}

void Effects::setRect(QRect area)
{
    QRect inWindowRect = area.intersected(QRect(0, 0, m_view->width(), m_view->height()));

    if (m_rect == inWindowRect) {
        return;
    }

    m_rect = inWindowRect;
    emit rectChanged();
}

QRect Effects::mask() const
{
    return m_mask;
}

void Effects::setMask(QRect area)
{
    if (m_mask == area)
        return;

    m_mask = area;

    if (KWindowSystem::compositingActive()) {
        if (m_view->behaveAsPlasmaPanel()) {
            m_view->setMask(QRect());
        } else {
            m_view->setMask(m_mask);
        }
    } else {
        //! this is used when compositing is disabled and provides
        //! the correct way for the mask to be painted in order for
        //! rounded corners to be shown correctly
        //! the enabledBorders check was added because there was cases
        //! that the mask region wasnt calculated correctly after location changes
        if (!m_background || m_background->enabledBorders() != m_enabledBorders) {
            m_background = new Plasma::FrameSvg(this);
        }

        if (m_background->imagePath() != "opaque/dialogs/background") {
            m_background->setImagePath(QStringLiteral("opaque/dialogs/background"));
        }

        m_background->setEnabledBorders(m_enabledBorders);
        m_background->resizeFrame(area.size());
        QRegion fixedMask = m_background->mask();
        fixedMask.translate(m_mask.x(), m_mask.y());

        //! fix for KF5.32 that return empty QRegion's for the mask
        if (fixedMask.isEmpty()) {
            fixedMask = QRegion(m_mask);
        }

        m_view->setMask(fixedMask);
    }

    // qDebug() << "dock mask set:" << m_mask;
    emit maskChanged();
}

void Effects::updateEffects()
{
    //! Don't apply any effect before the wayland surface is created under wayland
    //! https://bugs.kde.org/show_bug.cgi?id=392890
    if (KWindowSystem::isPlatformWayland() && !m_view->surface()) {
        return;
    }

    if (!m_view->behaveAsPlasmaPanel()) {
        if (m_drawEffects && !m_rect.isNull() && !m_rect.isEmpty()) {
            //! this is used when compositing is disabled and provides
            //! the correct way for the mask to be painted in order for
            //! rounded corners to be shown correctly
            if (!m_background) {
                m_background = new Plasma::FrameSvg(this);
            }

            if (m_background->imagePath() != "widgets/panel-background") {
                m_background->setImagePath(QStringLiteral("widgets/panel-background"));
            }

            m_background->setEnabledBorders(m_enabledBorders);
            m_background->resizeFrame(m_rect.size());
            QRegion fixedMask = m_background->mask();
            fixedMask.translate(m_rect.x(), m_rect.y());

            //! fix1, for KF5.32 that return empty QRegion's for the mask
            if (fixedMask.isEmpty()) {
                fixedMask = QRegion(m_rect);
            }

            KWindowEffects::enableBlurBehind(m_view->winId(), true, fixedMask);

            bool drawBackgroundEffect = m_theme.backgroundContrastEnabled() && (m_backgroundOpacity == 100);
            //based on Breeze Dark theme behavior the enableBackgroundContrast even though it does accept
            //a QRegion it uses only the first rect. The bug was that for Breeze Dark there was a line
            //at the dock bottom that was distinguishing it from other themes
            KWindowEffects::enableBackgroundContrast(m_view->winId(), drawBackgroundEffect,
                    m_theme.backgroundContrast(),
                    m_theme.backgroundIntensity(),
                    m_theme.backgroundSaturation(),
                    fixedMask.boundingRect());
        } else {
            KWindowEffects::enableBlurBehind(m_view->winId(), false);
            KWindowEffects::enableBackgroundContrast(m_view->winId(), false);
        }
    } else if (m_view->behaveAsPlasmaPanel() && m_drawEffects) {
        KWindowEffects::enableBlurBehind(m_view->winId(), true);

        bool drawBackgroundEffect = m_theme.backgroundContrastEnabled() && (m_backgroundOpacity == 100);

        KWindowEffects::enableBackgroundContrast(m_view->winId(), drawBackgroundEffect,
                m_theme.backgroundContrast(),
                m_theme.backgroundIntensity(),
                m_theme.backgroundSaturation());
    } else {
        KWindowEffects::enableBlurBehind(m_view->winId(), false);
        KWindowEffects::enableBackgroundContrast(m_view->winId(), false);
    }
}

//!BEGIN draw panel shadows outside the dock window
Plasma::FrameSvg::EnabledBorders Effects::enabledBorders() const
{
    return m_enabledBorders;
}

void Effects::updateEnabledBorders()
{
    if (!m_view->screen()) {
        return;
    }

    Plasma::FrameSvg::EnabledBorders borders = Plasma::FrameSvg::AllBorders;

    switch (m_view->location()) {
        case Plasma::Types::TopEdge:
            borders &= ~Plasma::FrameSvg::TopBorder;
            break;

        case Plasma::Types::LeftEdge:
            borders &= ~Plasma::FrameSvg::LeftBorder;
            break;

        case Plasma::Types::RightEdge:
            borders &= ~Plasma::FrameSvg::RightBorder;
            break;

        case Plasma::Types::BottomEdge:
            borders &= ~Plasma::FrameSvg::BottomBorder;
            break;

        default:
            break;
    }

    if ((m_view->location() == Plasma::Types::LeftEdge || m_view->location() == Plasma::Types::RightEdge)) {
        if (m_view->maxLength() == 1 && m_view->alignment() == Latte::Dock::Justify && !m_forceDrawCenteredBorders) {
            borders &= ~Plasma::FrameSvg::TopBorder;
            borders &= ~Plasma::FrameSvg::BottomBorder;
        }

        if (m_view->alignment() == Latte::Dock::Top && !m_forceDrawCenteredBorders && m_view->offset() == 0) {
            borders &= ~Plasma::FrameSvg::TopBorder;
        }

        if (m_view->alignment() == Latte::Dock::Bottom && !m_forceDrawCenteredBorders && m_view->offset() == 0) {
            borders &= ~Plasma::FrameSvg::BottomBorder;
        }
    }

    if (m_view->location() == Plasma::Types::TopEdge || m_view->location() == Plasma::Types::BottomEdge) {
        if (m_view->maxLength() == 1 && m_view->alignment() == Latte::Dock::Justify) {
            borders &= ~Plasma::FrameSvg::LeftBorder;
            borders &= ~Plasma::FrameSvg::RightBorder;
        }

        if (m_view->alignment() == Latte::Dock::Left && m_view->offset() == 0) {
            borders &= ~Plasma::FrameSvg::LeftBorder;
        }

        if (m_view->alignment() == Latte::Dock::Right  && m_view->offset() == 0) {
            borders &= ~Plasma::FrameSvg::RightBorder;
        }
    }

    if (m_enabledBorders != borders) {
        m_enabledBorders = borders;
        emit enabledBordersChanged();
    }

    if (!m_view->behaveAsPlasmaPanel() || !m_drawShadows) {
        PanelShadows::self()->removeWindow(m_view);
    } else {
        PanelShadows::self()->setEnabledBorders(m_view, borders);
    }
}
//!END draw panel shadows outside the dock window

}
}
