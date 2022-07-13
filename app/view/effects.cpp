/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "effects.h"

// local
#include <config-latte.h>
#include <coretypes.h>
#include "panelshadows_p.h"
#include "view.h"
#include "../lattecorona.h"
#include "../wm/abstractwindowinterface.h"

// Qt
#include <QRegion>

// KDE
#include <KWindowEffects>
#include <KWindowSystem>


namespace Latte {
namespace ViewPart {

Effects::Effects(Latte::View *parent)
    : QObject(parent),
      m_view(parent)
{
    m_corona = qobject_cast<Latte::Corona *>(m_view->corona());

    init();
}

Effects::~Effects()
{
}

void Effects::init()
{
    connect(this, &Effects::backgroundOpacityChanged, this, &Effects::updateEffects);
    connect(this, &Effects::backgroundOpacityChanged, this, &Effects::updateBackgroundContrastValues);
    connect(this, &Effects::backgroundCornersMaskChanged, this, &Effects::updateEffects);
    connect(this, &Effects::backgroundRadiusEnabledChanged, this, &Effects::updateEffects);
    connect(this, &Effects::drawEffectsChanged, this, &Effects::updateEffects);
    connect(this, &Effects::enabledBordersChanged, this, &Effects::updateEffects);
    connect(this, &Effects::rectChanged, this, &Effects::updateEffects);


    connect(this, &Effects::backgroundCornersMaskChanged, this, &Effects::updateMask);
    connect(this, &Effects::backgroundRadiusEnabledChanged, this, &Effects::updateMask);
    connect(this, &Effects::subtractedMaskRegionsChanged, this, &Effects::updateMask);
    connect(this, &Effects::unitedMaskRegionsChanged, this, &Effects::updateMask);
    connect(m_view, &QQuickWindow::widthChanged, this, &Effects::updateMask);
    connect(m_view, &QQuickWindow::heightChanged, this, &Effects::updateMask);
    connect(m_view, &Latte::View::behaveAsPlasmaPanelChanged, this, &Effects::updateMask);
    connect(KWindowSystem::self(), &KWindowSystem::compositingChanged, this, [&]() {
        if (!KWindowSystem::compositingActive() && !m_view->behaveAsPlasmaPanel()) {
            setMask(m_rect);
        }

        updateMask();
    });

    connect(this, &Effects::rectChanged, this, [&]() {
        if (!KWindowSystem::compositingActive() && !m_view->behaveAsPlasmaPanel()) {
            setMask(m_rect);
        }
    });

    connect(this, &Effects::backgroundRadiusChanged, this, &Effects::updateBackgroundCorners);

    connect(this, &Effects::drawShadowsChanged, this, [&]() {
        if (m_view->behaveAsPlasmaPanel()) {
            updateEnabledBorders();
        }
    });

    connect(this, &Effects::backgroundAllCornersChanged, this, &Effects::updateEnabledBorders);

    connect(this, &Effects::popUpMarginChanged, this, &Effects::onPopUpMarginChanged);

    connect(m_view, &Latte::View::alignmentChanged, this, &Effects::updateEnabledBorders);
    connect(m_view, &Latte::View::maxLengthChanged, this, &Effects::updateEnabledBorders);
    connect(m_view, &Latte::View::offsetChanged, this, &Effects::updateEnabledBorders);
    connect(m_view, &Latte::View::screenEdgeMarginEnabledChanged, this, &Effects::updateEnabledBorders);
    connect(m_view, &Latte::View::behaveAsPlasmaPanelChanged, this, &Effects::updateEffects);
    connect(this, &Effects::drawShadowsChanged, this, &Effects::updateShadows);
    connect(m_view, &Latte::View::behaveAsPlasmaPanelChanged, this, &Effects::updateShadows);
    connect(m_view, &Latte::View::configWindowGeometryChanged, this, &Effects::updateMask);
    connect(m_view, &Latte::View::layoutChanged, this, &Effects::onPopUpMarginChanged);

    connect(&m_theme, &Plasma::Theme::themeChanged, this, [&]() {
        updateBackgroundContrastValues();
        updateEffects();
    });
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

bool Effects::backgroundAllCorners() const
{
    return m_backgroundAllCorners;
}

void Effects::setBackgroundAllCorners(bool allcorners)
{
    if (m_backgroundAllCorners == allcorners) {
        return;
    }

    m_backgroundAllCorners = allcorners;
    emit backgroundAllCornersChanged();
}

bool Effects::backgroundRadiusEnabled() const
{
    return m_backgroundRadiusEnabled;
}

void Effects::setBackgroundRadiusEnabled(bool enabled)
{
    if (m_backgroundRadiusEnabled == enabled) {
        return;
    }

    m_backgroundRadiusEnabled = enabled;
    emit backgroundRadiusEnabledChanged();
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

void Effects::setForceBottomBorder(bool draw)
{
    if (m_forceBottomBorder == draw) {
        return;
    }

    m_forceBottomBorder = draw;
    updateEnabledBorders();
}

void Effects::setForceTopBorder(bool draw)
{
    if (m_forceTopBorder == draw) {
        return;
    }

    m_forceTopBorder = draw;
    updateEnabledBorders();
}

int Effects::backgroundRadius()
{
    return m_backgroundRadius;
}

void Effects::setBackgroundRadius(const int &radius)
{
    if (m_backgroundRadius == radius) {
        return;
    }

    m_backgroundRadius = radius;
    emit backgroundRadiusChanged();
}

float Effects::backgroundOpacity() const
{
    return m_backgroundOpacity;
}

void Effects::setBackgroundOpacity(float opacity)
{
    if (m_backgroundOpacity == opacity) {
        return;
    }

    m_backgroundOpacity = opacity;

    updateBackgroundContrastValues();
    emit backgroundOpacityChanged();
}

int Effects::editShadow() const
{
    return m_editShadow;
}

void Effects::setEditShadow(int shadow)
{
    if (m_editShadow == shadow) {
        return;
    }

    m_editShadow = shadow;
    emit editShadowChanged();
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

int Effects::popUpMargin() const
{
    return m_view->layout() ? m_view->layout()->popUpMargin() : -1/*default*/;
}

QRect Effects::rect() const
{
    return m_rect;
}

void Effects::setRect(QRect area)
{
    if (m_rect == area) {
        return;
    }

    m_rect = area;

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
    updateMask();

    // qDebug() << "dock mask set:" << m_mask;
    emit maskChanged();
}

QRect Effects::inputMask() const
{
    return m_inputMask;
}

void Effects::setInputMask(QRect area)
{
    if (m_inputMask == area) {
        return;
    }

    m_inputMask = area;

    if (KWindowSystem::isPlatformX11()) {
        if (m_view->devicePixelRatio() != 1.0) {
            //!Fix for X11 Global Scale
            auto ratio = m_view->devicePixelRatio();
            area = QRect(qRound(area.x() * ratio),
                         qRound(area.y() * ratio),
                         qRound(area.width()*ratio),
                         qRound(area.height() * ratio));
        }

        m_corona->wm()->setInputMask(m_view, area);
    } else {
        //under wayland mask() is providing the Input Area
        m_view->setMask(area);
    }

    emit inputMaskChanged();
}

QRect Effects::appletsLayoutGeometry() const
{
    return m_appletsLayoutGeometry;
}

void Effects::setAppletsLayoutGeometry(const QRect &geom)
{
    if (m_appletsLayoutGeometry == geom) {
        return;
    }

    m_appletsLayoutGeometry = geom;
    m_view->setProperty("_applets_layout_geometry", QVariant(m_appletsLayoutGeometry));

    emit appletsLayoutGeometryChanged();
}

QQuickItem *Effects::panelBackgroundSvg() const
{
    return m_panelBackgroundSvg;
}

void Effects::setPanelBackgroundSvg(QQuickItem *quickitem)
{
    if (m_panelBackgroundSvg == quickitem) {
        return;
    }

    m_panelBackgroundSvg = quickitem;
    emit panelBackgroundSvgChanged();
}

void Effects::onPopUpMarginChanged()
{
    m_view->setProperty("_applets_popup_margin", QVariant(popUpMargin()));
}

void Effects::forceMaskRedraw()
{
    updateMask();
}

void Effects::setSubtractedMaskRegion(const QString &regionid, const QRegion &region)
{
    if (m_subtractedMaskRegions.contains(regionid) && m_subtractedMaskRegions[regionid] == region) {
        return;
    }

    m_subtractedMaskRegions[regionid] = region;
    emit subtractedMaskRegionsChanged();
}

void Effects::removeSubtractedMaskRegion(const QString &regionid)
{
    if (!m_subtractedMaskRegions.contains(regionid)) {
        return;
    }

    m_subtractedMaskRegions.remove(regionid);
    emit subtractedMaskRegionsChanged();
}

void Effects::setUnitedMaskRegion(const QString &regionid, const QRegion &region)
{
    if (m_unitedMaskRegions.contains(regionid) && m_unitedMaskRegions[regionid] == region) {
        return;
    }

    m_unitedMaskRegions[regionid] = region;
    emit unitedMaskRegionsChanged();
}

void Effects::removeUnitedMaskRegion(const QString &regionid)
{
    if (!m_unitedMaskRegions.contains(regionid)) {
        return;
    }

    m_unitedMaskRegions.remove(regionid);
    emit unitedMaskRegionsChanged();
}

QRegion Effects::customMask(const QRect &rect)
{
    QRegion result = rect;
    int dx = rect.right() - m_cornersMaskRegion.topLeft.boundingRect().width() + 1;
    int dy = rect.bottom() - m_cornersMaskRegion.topLeft.boundingRect().height() + 1;

    if (m_hasTopLeftCorner) {
        QRegion tl = m_cornersMaskRegion.topLeft;
        tl.translate(rect.x(), rect.y());
        result = result.subtracted(tl);
    }

    if (m_hasTopRightCorner) {
        QRegion tr = m_cornersMaskRegion.topRight;
        tr.translate(rect.x() + dx, rect.y());
        result = result.subtracted(tr);
    }

    if (m_hasBottomRightCorner) {
        QRegion br = m_cornersMaskRegion.bottomRight;
        br.translate(rect.x() + dx, rect.y() + dy);
        result = result.subtracted(br);
    }

    if (m_hasBottomLeftCorner) {
        QRegion bl = m_cornersMaskRegion.bottomLeft;
        bl.translate(rect.x(), rect.y() + dy);
        result = result.subtracted(bl);
    }

    return result;
}

QRegion Effects::maskCombinedRegion()
{
    QRegion region = m_mask;

    for(auto subregion : m_subtractedMaskRegions) {
        region = region.subtracted(subregion);
    }

    for(auto subregion : m_unitedMaskRegions) {
        region = region.united(subregion);
    }

    return region;
}

void Effects::updateBackgroundCorners()
{
    if (m_backgroundRadius<0) {
        return;
    }

    m_corona->themeExtended()->cornersMask(m_backgroundRadius);

    m_cornersMaskRegion = m_corona->themeExtended()->cornersMask(m_backgroundRadius);
    emit backgroundCornersMaskChanged();
}

void Effects::updateMask()
{
    if (KWindowSystem::compositingActive()) {
        if (KWindowSystem::isPlatformX11()) {
            if (m_view->behaveAsPlasmaPanel()) {
                // set as NULL in order for plasma framrworks to identify NULL Mask properly
                m_view->setMask(QRect(-1, -1, 0, 0));
            } else {
                m_view->setMask(QRect(0, 0, m_view->width(), m_view->height()));
            }
        } else {
            // under wayland do nothing
        }
    } else {
        QRegion fixedMask;

        QRect maskRect = m_view->behaveAsPlasmaPanel() ? QRect(0,0, m_view->width(), m_view->height()) : m_mask;

        if (m_backgroundRadiusEnabled) {
            //! CustomBackground way
            fixedMask = customMask(QRect(0,0,maskRect.width(), maskRect.height()));
        } else {
            //! Plasma::Theme way
            //! this is used when compositing is disabled and provides
            //! the correct way for the mask to be painted in order for
            //! rounded corners to be shown correctly
            //! the enabledBorders check was added because there was cases
            //! that the mask region wasn't calculated correctly after location changes
            if (!m_panelBackgroundSvg) {
                return;
            }

            const QVariant maskProperty = m_panelBackgroundSvg->property("mask");
            if (static_cast<QMetaType::Type>(maskProperty.type()) == QMetaType::QRegion) {
                fixedMask = maskProperty.value<QRegion>();
            }
        }

        fixedMask.translate(maskRect.x(), maskRect.y());

        //! fix for KF5.32 that return empty QRegion's for the mask
        if (fixedMask.isEmpty()) {
            fixedMask = QRegion(maskRect);
        }

        m_view->setMask(fixedMask);
    }
}

void Effects::clearShadows()
{
    PanelShadows::self()->removeWindow(m_view);
}

void Effects::updateShadows()
{
    if (m_view->behaveAsPlasmaPanel() && drawShadows()) {
        PanelShadows::self()->addWindow(m_view, enabledBorders());
    } else {
        PanelShadows::self()->removeWindow(m_view);
    }
}

void Effects::updateEffects()
{
    //! Don't apply any effect before the wayland surface is created under wayland
    //! https://bugs.kde.org/show_bug.cgi?id=392890
    if (KWindowSystem::isPlatformWayland() && !m_view->surface()) {
        return;
    }

    bool clearEffects{true};

    if (m_drawEffects) {
        if (!m_view->behaveAsPlasmaPanel()) {
            if (!m_rect.isNull() && !m_rect.isEmpty() && m_rect != VisibilityManager::ISHIDDENMASK) {
                QRegion backMask;

                if (m_backgroundRadiusEnabled) {
                    //! CustomBackground way
                    backMask = customMask(QRect(0,0,m_rect.width(), m_rect.height()));
                } else {
                    //! Plasma::Theme way
                    //! this is used when compositing is disabled and provides
                    //! the correct way for the mask to be painted in order for
                    //! rounded corners to be shown correctly
                    if (!m_panelBackgroundSvg) {
                        return;
                    }

                    if (m_rect == VisibilityManager::ISHIDDENMASK) {
                        clearEffects = true;
                    } else {
                        const QVariant maskProperty = m_panelBackgroundSvg->property("mask");
                        if (static_cast<QMetaType::Type>(maskProperty.type()) == QMetaType::QRegion) {
                            backMask = maskProperty.value<QRegion>();
                        }
                    }
                }

                //! adjust mask coordinates based on local coordinates
                int fX = m_rect.x(); int fY = m_rect.y();

                //! Latte is now using GtkFrameExtents so Effects geometries must be adjusted
                //! windows that use GtkFrameExtents and apply Effects on them they take GtkFrameExtents
                //! as granted
                if (KWindowSystem::isPlatformX11() && !m_view->byPassWM()) {
                    if (m_view->location() == Plasma::Types::BottomEdge) {
                        fY = qMax(0, fY - m_view->headThicknessGap());
                    } else if (m_view->location() == Plasma::Types::RightEdge) {
                        fX = qMax(0, fX - m_view->headThicknessGap());
                    }
                }

                //! There are cases that mask is NULL even though it should not
                //! Example: SidebarOnDemand from v0.10 that BEHAVEASPLASMAPANEL in EditMode
                //! switching multiple times between inConfigureAppletsMode and LiveEditMode
                //! is such a case
                QRegion fixedMask;

                if (!backMask.isNull()) {
                    fixedMask = backMask;
                    fixedMask.translate(fX, fY);
                } else {
                    fixedMask = QRect(fX, fY, m_rect.width(), m_rect.height());
                }

                if (!fixedMask.isEmpty()) {
                    clearEffects = false;
                    KWindowEffects::enableBlurBehind(m_view->winId(), true, fixedMask);
                    KWindowEffects::enableBackgroundContrast(m_view->winId(),
                                                             m_theme.backgroundContrastEnabled(),
                                                             m_backEffectContrast,
                                                             m_backEffectIntesity,
                                                             m_backEffectSaturation,
                                                             fixedMask);
                }
            }
        } else {
            //!  BEHAVEASPLASMAPANEL case
            clearEffects = false;
            KWindowEffects::enableBlurBehind(m_view->winId(), true);
            KWindowEffects::enableBackgroundContrast(m_view->winId(),
                                                     m_theme.backgroundContrastEnabled(),
                                                     m_backEffectContrast,
                                                     m_backEffectIntesity,
                                                     m_backEffectSaturation);
        }
    }

    if (clearEffects) {
        KWindowEffects::enableBlurBehind(m_view->winId(), false);
        KWindowEffects::enableBackgroundContrast(m_view->winId(), false);
    }
}

//!BEGIN draw panel shadows outside the dock window
Plasma::FrameSvg::EnabledBorders Effects::enabledBorders() const
{
    return m_enabledBorders;
}

qreal Effects::currentMidValue(const qreal &max, const qreal &factor, const qreal &min) const
{
    if (max==min || factor==0) {
        return min;
    }

    qreal space = 0;
    qreal distance = 0;

    if (max<min) {
        space = min-max;
        distance = factor*space;
        return 1-distance;
    } else {
        space = max-min;
        distance = factor*space;
        return 1+distance;
    }
}

void Effects::updateBackgroundContrastValues()
{
    if (!m_theme.backgroundContrastEnabled()) {
        m_backEffectContrast = 1.0;
        m_backEffectIntesity = 1.0;
        m_backEffectSaturation = 1.0;
        return;
    }

    if (m_backgroundOpacity == -1 /*Default plasma opacity option*/) {
        m_backEffectContrast = m_theme.backgroundContrast();
        m_backEffectIntesity = m_theme.backgroundIntensity();
        m_backEffectSaturation = m_theme.backgroundSaturation();
    } else {
        m_backEffectContrast = currentMidValue(m_theme.backgroundContrast(), m_backgroundOpacity, 1);
        m_backEffectIntesity = currentMidValue(m_theme.backgroundIntensity(), m_backgroundOpacity, 1);
        m_backEffectSaturation = currentMidValue(m_theme.backgroundSaturation(), m_backgroundOpacity, 1);
    }
}

void Effects::updateEnabledBorders()
{
    if (!m_view->screen()) {
        return;
    }

    Plasma::FrameSvg::EnabledBorders borders = Plasma::FrameSvg::AllBorders;

    if (!m_view->screenEdgeMarginEnabled() && !m_backgroundAllCorners) {
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
    }

    if (!m_backgroundAllCorners) {
        if ((m_view->location() == Plasma::Types::LeftEdge || m_view->location() == Plasma::Types::RightEdge)) {
            if (m_view->maxLength() == 1 && m_view->alignment() == Latte::Types::Justify) {
                if (!m_forceTopBorder) {
                    borders &= ~Plasma::FrameSvg::TopBorder;
                }

                if (!m_forceBottomBorder) {
                    borders &= ~Plasma::FrameSvg::BottomBorder;
                }
            }

            if (m_view->alignment() == Latte::Types::Top && !m_forceTopBorder && m_view->offset() == 0) {
                borders &= ~Plasma::FrameSvg::TopBorder;
            }

            if (m_view->alignment() == Latte::Types::Bottom && !m_forceBottomBorder && m_view->offset() == 0) {
                borders &= ~Plasma::FrameSvg::BottomBorder;
            }
        }

        if (m_view->location() == Plasma::Types::TopEdge || m_view->location() == Plasma::Types::BottomEdge) {
            if (m_view->maxLength() == 1 && m_view->alignment() == Latte::Types::Justify) {
                borders &= ~Plasma::FrameSvg::LeftBorder;
                borders &= ~Plasma::FrameSvg::RightBorder;
            }

            if (m_view->alignment() == Latte::Types::Left && m_view->offset() == 0) {
                borders &= ~Plasma::FrameSvg::LeftBorder;
            }

            if (m_view->alignment() == Latte::Types::Right  && m_view->offset() == 0) {
                borders &= ~Plasma::FrameSvg::RightBorder;
            }
        }
    }

    m_hasTopLeftCorner =  (borders == Plasma::FrameSvg::AllBorders) || ((borders & Plasma::FrameSvg::TopBorder) && (borders & Plasma::FrameSvg::LeftBorder));
    m_hasTopRightCorner =  (borders == Plasma::FrameSvg::AllBorders) || ((borders & Plasma::FrameSvg::TopBorder) && (borders & Plasma::FrameSvg::RightBorder));
    m_hasBottomLeftCorner =  (borders == Plasma::FrameSvg::AllBorders) || ((borders & Plasma::FrameSvg::BottomBorder) && (borders & Plasma::FrameSvg::LeftBorder));
    m_hasBottomRightCorner =  (borders == Plasma::FrameSvg::AllBorders) || ((borders & Plasma::FrameSvg::BottomBorder) && (borders & Plasma::FrameSvg::RightBorder));

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
