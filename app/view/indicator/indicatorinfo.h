/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

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
    void providesFrontLayerChanged();

private:
    bool m_needsIconColors{false};
    bool m_needsMouseEventCoordinates{false};
    bool m_providesClickedAnimation{false};
    bool m_providesHoveredAnimation{false};
    bool m_providesFrontLayer{false};

    int m_extraMaskThickness{0};

    float m_minLengthPadding{0};
    float m_minThicknessPadding{0};
};

}
}
}

#endif
