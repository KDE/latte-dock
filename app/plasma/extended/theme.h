/*
 * Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
    void updateReversedScheme();
    void updateReversedSchemeValues();

    void qmlRegisterTypes();

private:
    bool m_hasShadow{false};
    bool m_isLightTheme{false};
    bool m_compositing{true};

    int m_outlineWidth{1};

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
