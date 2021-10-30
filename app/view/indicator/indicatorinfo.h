/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWINDICATORINFO_H
#define VIEWINDICATORINFO_H

// Qt
#include <QObject>

namespace Latte {
namespace ViewPart {
namespace IndicatorPart {

/**
 * Information provided by indicator itself in order to provide a nice experience
 **/

class Info: public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool needsIconColors READ needsIconColors WRITE setNeedsIconColors NOTIFY needsIconColorsChanged)
    Q_PROPERTY(bool needsMouseEventCoordinates READ needsMouseEventCoordinates WRITE setNeedsMouseEventCoordinates NOTIFY needsMouseEventCoordinatesChanged)

    Q_PROPERTY(bool providesClickedAnimation READ providesClickedAnimation WRITE setProvidesClickedAnimation NOTIFY providesClickedAnimationChanged)
    Q_PROPERTY(bool providesHoveredAnimation READ providesHoveredAnimation WRITE setProvidesHoveredAnimation NOTIFY providesHoveredAnimationChanged)
    Q_PROPERTY(bool providesInAttentionAnimation READ providesInAttentionAnimation WRITE setProvidesInAttentionAnimation NOTIFY providesInAttentionAnimationChanged)
    Q_PROPERTY(bool providesTaskLauncherAnimation READ providesTaskLauncherAnimation WRITE setProvidesTaskLauncherAnimation NOTIFY providesTaskLauncherAnimationChanged)
    Q_PROPERTY(bool providesGroupedWindowAddedAnimation READ providesGroupedWindowAddedAnimation WRITE setProvidesGroupedWindowAddedAnimation NOTIFY providesGroupedWindowAddedAnimationChanged)
    Q_PROPERTY(bool providesGroupedWindowRemovedAnimation READ providesGroupedWindowRemovedAnimation WRITE setProvidesGroupedWindowRemovedAnimation NOTIFY providesGroupedWindowRemovedAnimationChanged)

    Q_PROPERTY(bool providesFrontLayer READ providesFrontLayer WRITE setProvidesFrontLayer NOTIFY providesFrontLayerChanged)

    Q_PROPERTY(int extraMaskThickness READ extraMaskThickness WRITE setExtraMaskThickness NOTIFY extraMaskThicknessChanged)

    Q_PROPERTY(float minLengthPadding READ minLengthPadding WRITE setMinLengthPadding NOTIFY minLengthPaddingChanged)
    Q_PROPERTY(float minThicknessPadding READ minThicknessPadding WRITE setMinThicknessPadding NOTIFY minThicknessPaddingChanged)

public:
    Info(QObject *parent);
    virtual ~Info();

    bool needsIconColors() const;
    void setNeedsIconColors(bool needs);

    bool needsMouseEventCoordinates() const;
    void setNeedsMouseEventCoordinates(bool needs);

    bool providesClickedAnimation() const;
    void setProvidesClickedAnimation(bool provides);

    bool providesHoveredAnimation() const;
    void setProvidesHoveredAnimation(bool provides);

    bool providesInAttentionAnimation() const;
    void setProvidesInAttentionAnimation(bool provides);

    bool providesTaskLauncherAnimation() const;
    void setProvidesTaskLauncherAnimation(bool provides);

    bool providesGroupedWindowAddedAnimation() const;
    void setProvidesGroupedWindowAddedAnimation(bool provides);

    bool providesGroupedWindowRemovedAnimation() const;
    void setProvidesGroupedWindowRemovedAnimation(bool provides);

    bool providesFrontLayer() const;
    void setProvidesFrontLayer(bool front);

    int extraMaskThickness() const;
    void setExtraMaskThickness(int thick);

    float minLengthPadding() const;
    void setMinLengthPadding(float padding);

    float minThicknessPadding() const;
    void setMinThicknessPadding(float padding);

signals:
    void extraMaskThicknessChanged();
    void minLengthPaddingChanged();
    void minThicknessPaddingChanged();
    void needsIconColorsChanged();
    void needsMouseEventCoordinatesChanged();
    void providesClickedAnimationChanged();
    void providesHoveredAnimationChanged();
    void providesInAttentionAnimationChanged();
    void providesTaskLauncherAnimationChanged();
    void providesGroupedWindowAddedAnimationChanged();
    void providesGroupedWindowRemovedAnimationChanged();
    void providesFrontLayerChanged();

private:
    bool m_needsIconColors{false};
    bool m_needsMouseEventCoordinates{false};
    bool m_providesClickedAnimation{false};
    bool m_providesHoveredAnimation{false};
    bool m_providesInAttentionAnimation{false};
    bool m_providesTaskLauncherAnimation{false};
    bool m_providesGroupedWindowAddedAnimation{false};
    bool m_providesGroupedWindowRemovedAnimation{false};
    bool m_providesFrontLayer{false};

    int m_extraMaskThickness{0};

    float m_minLengthPadding{0};
    float m_minThicknessPadding{0};
};

}
}
}

#endif
