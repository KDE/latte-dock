/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "indicatorinfo.h"


namespace Latte {
namespace ViewPart {
namespace IndicatorPart {

Info::Info(QObject *parent) :
    QObject(parent)
{
}

Info::~Info()
{
}

bool Info::needsIconColors() const
{
    return m_needsIconColors;
}

void Info::setNeedsIconColors(bool needs)
{
    if (m_needsIconColors == needs) {
        return;
    }

    m_needsIconColors = needs;
    emit needsIconColorsChanged();
}

bool Info::needsMouseEventCoordinates() const
{
    return m_needsMouseEventCoordinates;
}

void Info::setNeedsMouseEventCoordinates(bool needs)
{
    if (m_needsMouseEventCoordinates == needs) {
        return;
    }

    m_needsMouseEventCoordinates = needs;
    emit needsMouseEventCoordinatesChanged();
}

bool Info::providesClickedAnimation() const
{
    return m_providesClickedAnimation;
}

void Info::setProvidesClickedAnimation(bool provides)
{
    if (m_providesClickedAnimation == provides) {
        return;
    }

    m_providesClickedAnimation = provides;
    emit providesClickedAnimationChanged();
}

bool Info::providesHoveredAnimation() const
{
    return m_providesHoveredAnimation;
}

void Info::setProvidesHoveredAnimation(bool provides)
{
    if (m_providesHoveredAnimation == provides) {
        return;
    }

    m_providesHoveredAnimation = provides;
    emit providesHoveredAnimationChanged();
}

bool Info::providesFrontLayer() const
{
    return m_providesFrontLayer;
}

void Info::setProvidesFrontLayer(bool front)
{
    if (m_providesFrontLayer == front) {
        return;
    }

    m_providesFrontLayer = front;
    emit providesFrontLayerChanged();
}

int Info::extraMaskThickness() const
{
    return m_extraMaskThickness;
}

void Info::setExtraMaskThickness(int thick)
{
    if (m_extraMaskThickness == thick) {
        return;
    }

    m_extraMaskThickness = thick;
    emit extraMaskThicknessChanged();
}

float Info::minLengthPadding() const
{
    return m_minLengthPadding;
}

void Info::setMinLengthPadding(float padding)
{
    if (m_minLengthPadding == padding) {
        return;
    }

    m_minLengthPadding = padding;
    emit minLengthPaddingChanged();
}

float Info::minThicknessPadding() const
{
    return m_minThicknessPadding;
}

void Info::setMinThicknessPadding(float padding)
{
    if (m_minThicknessPadding == padding) {
        return;
    }

    m_minThicknessPadding = padding;
    emit minThicknessPaddingChanged();
}

}
}
}
