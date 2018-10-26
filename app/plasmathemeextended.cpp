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

#include "dockcorona.h"
#include "plasmathemeextended.h"

#include <QDebug>
#include <KSharedConfig>

namespace Latte {

PlasmaThemeExtended::PlasmaThemeExtended(KSharedConfig::Ptr config, QObject *parent) :
    QObject(parent),
    m_themeGroup(KConfigGroup(config, QStringLiteral("PlasmaThemeExtended")))
{
    m_corona = qobject_cast<DockCorona *>(parent);

    loadConfig();

    connect(&m_theme, &Plasma::Theme::themeChanged, this, &PlasmaThemeExtended::load);
}

void PlasmaThemeExtended::load()
{
    loadRoundness();
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
    return m_themeHasExtendedInfo;
}

void PlasmaThemeExtended::loadRoundness()
{
    if (!m_corona) {
        return;
    }

    QString extendedInfoFilePath = m_corona->kPackage().filePath("themesExtendedInfo");

    KSharedConfigPtr extInfoPtr = KSharedConfig::openConfig(extendedInfoFilePath);
    KConfigGroup roundGroup(extInfoPtr, "Roundness");

    m_themeHasExtendedInfo = false;

    qDebug() << "current theme ::: " << m_theme.themeName();

    foreach (auto key, roundGroup.keyList()) {
        qDebug() << "key ::: " << key;

        if (m_theme.themeName().toUpper().startsWith(key.toUpper())) {
            QStringList rs = roundGroup.readEntry(key, QStringList());
            qDebug() << "rounds ::: " << rs;

            if (rs.size() > 0) {
                m_themeHasExtendedInfo = true;

                if (rs.size() <= 3) {
                    //assign same roundness for all edges
                    m_bottomEdgeRoundness = rs[0].toInt();
                    m_leftEdgeRoundness = m_bottomEdgeRoundness;
                    m_topEdgeRoundness = m_bottomEdgeRoundness;
                    m_rightEdgeRoundness = m_bottomEdgeRoundness;
                } else if (rs.size() >= 4) {
                    m_bottomEdgeRoundness = rs[0].toInt();
                    m_leftEdgeRoundness = rs[1].toInt();
                    m_topEdgeRoundness = rs[2].toInt();
                    m_rightEdgeRoundness = rs[3].toInt();
                }
            }

            break;
        }
    }

    emit roundnessChanged();
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
