/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLASMATHEMEEXTENDED_H
#define PLASMATHEMEEXTENDED_H

// C++
#include <array>

// Qt
#include <QObject>
#include <QHash>
#include <QTemporaryDir>

// KDE
#include <KConfigGroup>
#include <KSharedConfig>

// Plasma
#include <Plasma/FrameSvg>
#include <Plasma/Theme>

namespace Latte {
class Corona;
namespace WindowSystem {
class SchemeColors;
}
}

namespace Latte {
namespace PlasmaExtended {
class PanelBackground;
}
}

namespace Latte {
namespace PlasmaExtended {

struct CornerRegions {
    QRegion topLeft;
    QRegion topRight;
    QRegion bottomLeft;
    QRegion bottomRight;
};

class Theme: public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasShadow READ hasShadow NOTIFY hasShadowChanged)
    Q_PROPERTY(bool isLightTheme READ isLightTheme NOTIFY themeChanged)
    Q_PROPERTY(bool isDarkTheme READ isDarkTheme NOTIFY themeChanged)

    Q_PROPERTY(int outlineWidth READ outlineWidth NOTIFY outlineWidthChanged)

    Q_PROPERTY(int marginsAreaTop READ marginsAreaTop NOTIFY marginsAreaChanged)
    Q_PROPERTY(int marginsAreaLeft READ marginsAreaLeft NOTIFY marginsAreaChanged)
    Q_PROPERTY(int marginsAreaBottom READ marginsAreaBottom NOTIFY marginsAreaChanged)
    Q_PROPERTY(int marginsAreaRight READ marginsAreaRight NOTIFY marginsAreaChanged)

    Q_PROPERTY(Latte::PlasmaExtended::PanelBackground *backgroundTopEdge READ backgroundTopEdge NOTIFY backgroundsChanged)
    Q_PROPERTY(Latte::PlasmaExtended::PanelBackground *backgroundLeftEdge READ backgroundLeftEdge NOTIFY backgroundsChanged)
    Q_PROPERTY(Latte::PlasmaExtended::PanelBackground *backgroundBottomEdge READ backgroundBottomEdge NOTIFY backgroundsChanged)
    Q_PROPERTY(Latte::PlasmaExtended::PanelBackground *backgroundRightEdge READ backgroundRightEdge NOTIFY backgroundsChanged)

    Q_PROPERTY(Latte::WindowSystem::SchemeColors *defaultTheme READ defaultTheme NOTIFY themeChanged)
    Q_PROPERTY(Latte::WindowSystem::SchemeColors *lightTheme READ lightTheme NOTIFY themeChanged)
    Q_PROPERTY(Latte::WindowSystem::SchemeColors *darkTheme READ darkTheme NOTIFY themeChanged)

public:
    Theme(KSharedConfig::Ptr config, QObject *parent);
    ~Theme() override;;

    bool hasShadow() const;
    bool isLightTheme() const;
    bool isDarkTheme() const;

    int outlineWidth() const;
    void setOutlineWidth(int width);

    int marginsAreaTop() const;
    int marginsAreaLeft() const;
    int marginsAreaBottom() const;
    int marginsAreaRight() const;

    PanelBackground *backgroundTopEdge() const;
    PanelBackground *backgroundLeftEdge() const;
    PanelBackground *backgroundBottomEdge() const;
    PanelBackground *backgroundRightEdge() const;

    WindowSystem::SchemeColors *defaultTheme() const;
    WindowSystem::SchemeColors *lightTheme() const;
    WindowSystem::SchemeColors *darkTheme() const;

    const CornerRegions &cornersMask(const int &radius);

    void load();

signals:
    void backgroundsChanged();
    void compositingChanged();
    void hasShadowChanged();
    void outlineWidthChanged();
    void marginsAreaChanged();
    void themeChanged();

private slots:
    void loadConfig();
    void saveConfig();
    void loadThemeLightness();

private:
    void loadThemePaths();
    void loadCompositingRoundness();
    void updateBackgrounds();

    void setOriginalSchemeFile(const QString &file);
    void updateHasShadow();
    void updateDefaultScheme();
    void updateDefaultSchemeValues();
    void updateMarginsAreaValues();
    void updateReversedScheme();
    void updateReversedSchemeValues();

    void qmlRegisterTypes();

private:
    bool m_hasShadow{false};
    bool m_isLightTheme{false};
    bool m_compositing{true};

    int m_outlineWidth{1};

    int m_marginsAreaTop{0};
    int m_marginsAreaLeft{0};
    int m_marginsAreaBottom{0};
    int m_marginsAreaRight{0};

    QString m_themePath;
    QString m_themeWidgetsPath;
    QString m_defaultSchemePath;
    QString m_originalSchemePath;
    QString m_reversedSchemePath;

    QHash<int, CornerRegions> m_cornerRegions;

    std::array<QMetaObject::Connection, 2> m_kdeConnections;

    QTemporaryDir m_extendedThemeDir;
    KConfigGroup m_themeGroup;
    Plasma::Theme m_theme;

    PanelBackground *m_backgroundTopEdge{nullptr};
    PanelBackground *m_backgroundLeftEdge{nullptr};
    PanelBackground *m_backgroundBottomEdge{nullptr};
    PanelBackground *m_backgroundRightEdge{nullptr};

    Latte::Corona *m_corona{nullptr};
    WindowSystem::SchemeColors *m_defaultScheme{nullptr};
    WindowSystem::SchemeColors *m_reversedScheme{nullptr};
};

}
}

#endif
