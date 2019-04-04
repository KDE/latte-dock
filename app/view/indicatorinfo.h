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
    Q_PROPERTY(bool providesFrontLayer READ providesFrontLayer WRITE setProvidesFrontLayer NOTIFY providesFrontLayerChanged)

    Q_PROPERTY(int extraMaskThickness READ extraMaskThickness WRITE setExtraMaskThickness NOTIFY extraMaskThicknessChanged)

    Q_PROPERTY(float minLengthPadding READ minLengthPadding WRITE setMinLengthPadding NOTIFY minLengthPaddingChanged)
    Q_PROPERTY(float minThicknessPadding READ minThicknessPadding WRITE setMinThicknessPadding NOTIFY minThicknessPaddingChanged)

public:
    Info(QObject *parent);
    virtual ~Info();

    bool needsIconColors() const;
    void setNeedsIconColors(bool needs);

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
    void providesFrontLayerChanged();

private:
    bool m_needsIconColors{false};
    bool m_providesFrontLayer{false};

    int m_extraMaskThickness{0};

    float m_minLengthPadding{0};
    float m_minThicknessPadding{0};
};

}
}
}

#endif
