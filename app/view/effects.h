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

// Qt
#include <QObject>
#include <QPointer>
#include <QQuickView>
#include <QRect>

// Plasma
#include <Plasma/FrameSvg>
#include <Plasma/Theme>

namespace Latte {
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

    Q_PROPERTY(float backgroundOpacity READ backgroundOpacity WRITE setBackgroundOpacity NOTIFY backgroundOpacityChanged)

    Q_PROPERTY(QRect mask READ mask WRITE setMask NOTIFY maskChanged)
    Q_PROPERTY(QRect rect READ rect WRITE setRect NOTIFY rectChanged)

    Q_PROPERTY(Plasma::FrameSvg::EnabledBorders enabledBorders READ enabledBorders NOTIFY enabledBordersChanged)

public:
    Effects(Latte::View *parent);
    virtual ~Effects();

    bool animationsBlocked() const;
    void setAnimationsBlocked(bool blocked);

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

    float backgroundOpacity() const;
    void setBackgroundOpacity(float opacity);

    QRect mask() const;
    void setMask(QRect area);

    QRect rect() const;
    void setRect(QRect area);

    Plasma::FrameSvg::EnabledBorders enabledBorders() const;

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
    void backgroundOpacityChanged();
    void drawShadowsChanged();
    void drawEffectsChanged();
    void editShadowChanged();
    void enabledBordersChanged();
    void maskChanged();
    void innerShadowChanged();
    void rectChanged();

    void subtractedMaskRegionsChanged();
    void unitedMaskRegionsChanged();

private slots:
    void init();

    void updateBackgroundContrastValues();

private:
    qreal currentMidValue(const qreal &max, const qreal &factor, const qreal &min) const;
    QRegion maskCombinedRegion();

private:
    bool m_animationsBlocked{false};
    bool m_drawShadows{true};
    bool m_drawEffects{false};
    bool m_forceTopBorder{false};
    bool m_forceBottomBorder{false};

    int m_editShadow{0};
    int m_innerShadow{0};

    float m_backgroundOpacity{1.0};

    qreal m_backEffectContrast{1};
    qreal m_backEffectIntesity{1};
    qreal m_backEffectSaturation{1};

    QRect m_rect;
    QRect m_mask;

    QPointer<Latte::View> m_view;

    Plasma::Theme m_theme;
    //only for the mask on disabled compositing, not to actually paint
    Plasma::FrameSvg *m_background{nullptr};

    //only for the mask, not to actually paint
    Plasma::FrameSvg::EnabledBorders m_enabledBorders{Plasma::FrameSvg::AllBorders};

    //! Subtracted and United Mask regions
    QHash<QString, QRegion> m_subtractedMaskRegions;
    QHash<QString, QRegion> m_unitedMaskRegions;
};

}
}

#endif
