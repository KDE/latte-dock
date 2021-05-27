/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLASMATHEMEEXTENDEDPANELBACKGROUND_H
#define PLASMATHEMEEXTENDEDPANELBACKGROUND_H

// Qt
#include <QObject>

// Plasma
#include <Plasma>
#include <Plasma/FrameSvg>

namespace Latte {
namespace PlasmaExtended {
class Theme;
}
}

namespace Latte {
namespace PlasmaExtended {

class PanelBackground: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int paddingTop READ paddingTop NOTIFY paddingsChanged)
    Q_PROPERTY(int paddingLeft READ paddingLeft NOTIFY paddingsChanged)
    Q_PROPERTY(int paddingBottom READ paddingBottom NOTIFY paddingsChanged)
    Q_PROPERTY(int paddingRight READ paddingRight NOTIFY paddingsChanged)
    Q_PROPERTY(int shadowSize READ shadowSize NOTIFY shadowSizeChanged)

    Q_PROPERTY(int roundness READ roundness NOTIFY roundnessChanged)

    Q_PROPERTY(float maxOpacity READ maxOpacity NOTIFY maxOpacityChanged)

    Q_PROPERTY(QColor shadowColor READ shadowColor NOTIFY shadowColorChanged)

public:
    PanelBackground(Plasma::Types::Location edge, Theme *parent);
    ~PanelBackground();

    int paddingTop() const;
    int paddingLeft() const;
    int paddingBottom() const;
    int paddingRight() const;
    int shadowSize() const;

    int roundness() const;

    float maxOpacity() const;

    QColor shadowColor() const;

public slots:
    void update();

signals:
    void paddingsChanged();
    void roundnessChanged();
    void shadowColorChanged();
    void shadowSizeChanged();
    void maxOpacityChanged();

private:
    bool hasMask(Plasma::Svg *svg) const;

    QString prefixed(const QString &id);
    QString element(Plasma::Svg *svg, const QString &id);

    void updateMaxOpacity(Plasma::Svg *svg);
    void updatePaddings(Plasma::Svg *svg);
    void updateRoundness(Plasma::Svg *svg);
    void updateShadow(Plasma::Svg *svg);

    void updateRoundnessFromMask(Plasma::Svg *svg);
    void updateRoundnessFromShadows(Plasma::Svg *svg);
    void updateRoundnessFallback(Plasma::Svg *svg);

private:
    int m_paddingTop{0};
    int m_paddingLeft{0};
    int m_paddingBottom{0};
    int m_paddingRight{0};

    int m_shadowSize{0};
    int m_roundness{0};

    float m_maxOpacity{1.0};

    QColor m_shadowColor;

    Plasma::Types::Location m_location{Plasma::Types::BottomEdge};

    Theme *m_parentTheme{nullptr};
};

}
}

#endif
