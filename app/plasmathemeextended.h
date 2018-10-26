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

#include <QObject>

#include <KConfigGroup>
#include <KSharedConfig>

#include <Plasma/Theme>

namespace Latte {

class DockCorona;

class PlasmaThemeExtended: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int bottomEdgeRoundness READ bottomEdgeRoundness NOTIFY roundnessChanged)
    Q_PROPERTY(int leftEdgeRoundness READ leftEdgeRoundness NOTIFY roundnessChanged)
    Q_PROPERTY(int topEdgeRoundness READ topEdgeRoundness NOTIFY roundnessChanged)
    Q_PROPERTY(int rightEdgeRoundness READ rightEdgeRoundness NOTIFY roundnessChanged)

public:
    PlasmaThemeExtended(KSharedConfig::Ptr config, QObject *parent);
    ~PlasmaThemeExtended() override;;

    int bottomEdgeRoundness() const;
    int leftEdgeRoundness() const;
    int topEdgeRoundness() const;
    int rightEdgeRoundness() const;

    int userThemeRoundness() const;
    void setUserThemeRoundness(int roundness);

    void load();

signals:
    void roundnessChanged();

private slots:
    void loadConfig();
    void saveConfig();

private:
    void loadThemePath();
    void loadRoundness();

    bool themeHasExtendedInfo() const;

private:
    bool m_themeHasExtendedInfo{false};

    int m_bottomEdgeRoundness{0};
    int m_leftEdgeRoundness{0};
    int m_topEdgeRoundness{0};
    int m_rightEdgeRoundness{0};
    int m_userRoundness{0};

    QString m_themePath;

    KConfigGroup m_themeGroup;

    Plasma::Theme m_theme;

    DockCorona *m_corona;
};

}

#endif
