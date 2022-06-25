/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EFFECTS_H
#define EFFECTS_H

// local
#include "../plasma/extended/theme.h"

// Qt
#include <QObject>
#include <QPointer>
#include <QQuickView>
#include <QRect>

// Plasma
#include <Plasma/FrameSvg>
#include <Plasma/Theme>

namespace Latte {
class Corona;
class View;
}

namespace Latte {
namespace ViewPart {

class Effects: public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool animationsBlocked READ animationsBlocked NOTIFY animationsBlockedChanged)
    Q_PROPERTY(bool drawShadows READ drawShadows WRITE setDrawShadows NOTIFY drawShadowsChanged)
    Q_PROPERTY(bool drawEffects READ drawEffects WRITE setDrawEffects NOTIFY drawEffectsChanged)

    //! thickness shadow size when is drawn inside the window from qml
    Q_PROPERTY(int editShadow READ editShadow WRITE setEditShadow NOTIFY editShadowChanged)
    Q_PROPERTY(int innerShadow READ innerShadow WRITE setInnerShadow NOTIFY innerShadowChanged)

    Q_PROPERTY(bool backgroundAllCorners READ backgroundAllCorners WRITE setBackgroundAllCorners NOTIFY backgroundAllCornersChanged)
    Q_PROPERTY(bool backgroundRadiusEnabled READ backgroundRadiusEnabled WRITE setBackgroundRadiusEnabled NOTIFY backgroundRadiusEnabledChanged)
    Q_PROPERTY(int backgroundRadius READ backgroundRadius WRITE setBackgroundRadius NOTIFY backgroundRadiusChanged)
    Q_PROPERTY(float backgroundOpacity READ backgroundOpacity WRITE setBackgroundOpacity NOTIFY backgroundOpacityChanged)

    Q_PROPERTY(int popUpMargin READ popUpMargin NOTIFY popUpMarginChanged)

    Q_PROPERTY(QRect mask READ mask WRITE setMask NOTIFY maskChanged)
    Q_PROPERTY(QRect rect READ rect WRITE setRect NOTIFY rectChanged)
    Q_PROPERTY(QRect inputMask READ inputMask WRITE setInputMask NOTIFY inputMaskChanged)
    Q_PROPERTY(QRect appletsLayoutGeometry READ appletsLayoutGeometry WRITE setAppletsLayoutGeometry NOTIFY appletsLayoutGeometryChanged)

    Q_PROPERTY(Plasma::FrameSvg::EnabledBorders enabledBorders READ enabledBorders NOTIFY enabledBordersChanged)

    Q_PROPERTY(QQuickItem *panelBackgroundSvg READ panelBackgroundSvg WRITE setPanelBackgroundSvg NOTIFY panelBackgroundSvgChanged)

public:
    Effects(Latte::View *parent);
    virtual ~Effects();

    bool animationsBlocked() const;
    void setAnimationsBlocked(bool blocked);

    bool backgroundAllCorners() const;
    void setBackgroundAllCorners(bool allcorners);

    bool backgroundRadiusEnabled() const;
    void setBackgroundRadiusEnabled(bool enabled);

    bool drawShadows() const;
    void setDrawShadows(bool draw);

    bool drawEffects() const;
    void setDrawEffects(bool draw);

    void setForceTopBorder(bool draw);
    void setForceBottomBorder(bool draw);

    int editShadow() const;
    void setEditShadow(int shadow);

    int innerShadow() const;
    void setInnerShadow(int shadow);

    int backgroundRadius();
    void setBackgroundRadius(const int &radius);

    int popUpMargin() const;

    float backgroundOpacity() const;
    void setBackgroundOpacity(float opacity);

    QRect mask() const;
    void setMask(QRect area);

    QRect inputMask() const;
    void setInputMask(QRect area);

    QRect rect() const;
    void setRect(QRect area);

    QRect appletsLayoutGeometry() const;
    void setAppletsLayoutGeometry(const QRect &geom);

    Plasma::FrameSvg::EnabledBorders enabledBorders() const;

    QQuickItem *panelBackgroundSvg() const;
    void setPanelBackgroundSvg(QQuickItem *quickitem);

public slots:
    Q_INVOKABLE void forceMaskRedraw();
    Q_INVOKABLE void setSubtractedMaskRegion(const QString &regionid, const QRegion &region);
    Q_INVOKABLE void removeSubtractedMaskRegion(const QString &regionid);
    Q_INVOKABLE void setUnitedMaskRegion(const QString &regionid, const QRegion &region);
    Q_INVOKABLE void removeUnitedMaskRegion(const QString &regionid);

    void clearShadows();
    void updateShadows();
    void updateEffects();
    void updateEnabledBorders();
    void updateMask();

signals:
    void animationsBlockedChanged();
    void appletsLayoutGeometryChanged();
    void backgroundAllCornersChanged();
    void backgroundCornersMaskChanged();
    void backgroundOpacityChanged();
    void backgroundRadiusEnabledChanged();
    void backgroundRadiusChanged();
    void drawShadowsChanged();
    void drawEffectsChanged();
    void editShadowChanged();
    void enabledBordersChanged();
    void maskChanged();
    void innerShadowChanged();
    void inputMaskChanged();
    void panelBackgroundSvgChanged();
    void popUpMarginChanged();
    void rectChanged();

    void subtractedMaskRegionsChanged();
    void unitedMaskRegionsChanged();

private slots:
    void init();

    void onPopUpMarginChanged();

    void updateBackgroundContrastValues();
    void updateBackgroundCorners();

private:
    bool backgroundRadiusIsEnabled() const;
    qreal currentMidValue(const qreal &max, const qreal &factor, const qreal &min) const;
    QRegion customMask(const QRect &rect);
    QRegion maskCombinedRegion();

private:
    bool m_animationsBlocked{false};
    bool m_backgroundAllCorners{false};
    bool m_backgroundRadiusEnabled{false};
    bool m_drawShadows{true};
    bool m_drawEffects{false};
    bool m_forceTopBorder{false};
    bool m_forceBottomBorder{false};

    bool m_hasTopLeftCorner{false};
    bool m_hasTopRightCorner{false};
    bool m_hasBottomLeftCorner{false};
    bool m_hasBottomRightCorner{false};

    int m_editShadow{0};
    int m_innerShadow{0};

    int m_backgroundRadius{-1};
    float m_backgroundOpacity{1.0};

    qreal m_backEffectContrast{1};
    qreal m_backEffectIntesity{1};
    qreal m_backEffectSaturation{1};

    QRect m_rect;
    QRect m_mask;
    QRect m_inputMask;
    QRect m_appletsLayoutGeometry;

    QPointer<Latte::View> m_view;
    QPointer<Latte::Corona> m_corona;

    PlasmaExtended::CornerRegions m_cornersMaskRegion;

    Plasma::Theme m_theme;

    //only for the mask, not to actually paint
    Plasma::FrameSvg::EnabledBorders m_enabledBorders{Plasma::FrameSvg::AllBorders};

    //assigned from qml side in order to access the official panel background svg
    QQuickItem *m_panelBackgroundSvg{nullptr};

    //! Subtracted and United Mask regions
    QHash<QString, QRegion> m_subtractedMaskRegions;
    QHash<QString, QRegion> m_unitedMaskRegions;
};

}
}

#endif
