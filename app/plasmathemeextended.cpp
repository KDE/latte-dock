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

#include "plasmathemeextended.h"

namespace Latte {

PlasmaThemeExtended::PlasmaThemeExtended(KSharedConfig::Ptr config, QObject *parent) :
    QObject(parent),
    m_themeGroup(KConfigGroup(config, QStringLiteral("PlasmaThemeExtended")))
{
}

PlasmaThemeExtended::~PlasmaThemeExtended()
{
    saveConfig();
}

int PlasmaThemeExtended::bottomEdgeRoundness() const
{
    return (themeHasExtendedInfo() ? m_bottomEdgeRoundness : userThemeRoundness());
}

int PlasmaThemeExtended::leftEdgeRoundness() const
{
    return (themeHasExtendedInfo() ? m_leftEdgeRoundness : userThemeRoundness());
}

int PlasmaThemeExtended::topEdgeRoundness() const
{
    return (themeHasExtendedInfo() ? m_topEdgeRoundness : userThemeRoundness());
}

int PlasmaThemeExtended::rightEdgeRoundness() const
{
    return (themeHasExtendedInfo() ? m_rightEdgeRoundness : userThemeRoundness());
}

int PlasmaThemeExtended::userThemeRoundness() const
{
    return m_userRoundness;
}

void PlasmaThemeExtended::setUserThemeRoundness(int roundness)
{
    if (m_userRoundness == roundness) {
        return;
    }

    m_userRoundness = roundness;

    if (!themeHasExtendedInfo()) {
        emit roundnessChanged();
    }

    saveConfig();
}

bool PlasmaThemeExtended::themeHasExtendedInfo() const
{
    return false;
}

void PlasmaThemeExtended::loadConfig()
{
    m_userRoundness = m_themeGroup.readEntry("userSetPlasmaThemeRoundness", 0);
}

void PlasmaThemeExtended::saveConfig()
{
    m_themeGroup.writeEntry("userSetPlasmaThemeRoundness", m_userRoundness);

    m_themeGroup.sync();
}

}
