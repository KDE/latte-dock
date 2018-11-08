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

#include "commontools.h"
#include "dockcorona.h"
#include "schemecolors.h"

#include <QDebug>
#include <QDir>

#include <KDirWatch>
#include <KConfigGroup>
#include <KSharedConfig>

#define REVERSEDCOLORSCHEME "reversed.colors"

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
    loadThemePaths();
    loadRoundness();
}

PlasmaThemeExtended::~PlasmaThemeExtended()
{
    saveConfig();

    m_normalScheme->deleteLater();
    m_reversedScheme->deleteLater();
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

SchemeColors *PlasmaThemeExtended::lightTheme() const
{
    return m_isLightTheme ? m_normalScheme : m_reversedScheme;
}

SchemeColors *PlasmaThemeExtended::darkTheme() const
{
    return !m_isLightTheme ? m_normalScheme : m_reversedScheme;
}


void PlasmaThemeExtended::setNormalSchemeFile(const QString &file)
{
    if (m_normalSchemePath == file) {
        return;
    }

    m_normalSchemePath = file;

    if (m_normalScheme) {
        disconnect(m_normalScheme, &SchemeColors::colorsChanged, this, &PlasmaThemeExtended::loadThemeLightness);
        m_normalScheme->deleteLater();
    }

    m_normalScheme = new SchemeColors(this, m_normalSchemePath, true);
    connect(m_normalScheme, &SchemeColors::colorsChanged, this, &PlasmaThemeExtended::loadThemeLightness);

    qDebug() << "plasma theme normal colors ::: " << m_normalSchemePath;

    updateReversedScheme();

    loadThemeLightness();

    emit themesChanged();
}

void PlasmaThemeExtended::updateReversedScheme()
{
    QString reversedFilePath = m_extendedThemeDir.path() + "/" + REVERSEDCOLORSCHEME;

    QFile(m_normalSchemePath).copy(reversedFilePath);
    m_reversedSchemePath = reversedFilePath;

    updateReversedSchemeValues();

    if (m_reversedScheme) {
        m_reversedScheme->deleteLater();
    }

    m_reversedScheme = new SchemeColors(this, m_reversedSchemePath, true);

    qDebug() << "plasma theme reversed colors ::: " << m_reversedSchemePath;
}

void PlasmaThemeExtended::updateReversedSchemeValues()
{
    //! reverse values based on original scheme
    KSharedConfigPtr normalPtr = KSharedConfig::openConfig(m_normalSchemePath);
    KSharedConfigPtr reversedPtr = KSharedConfig::openConfig(m_reversedSchemePath);

    if (normalPtr && reversedPtr) {
        foreach (auto groupName, reversedPtr->groupList()) {
            KConfigGroup reversedGroup(reversedPtr, groupName);

            if (reversedGroup.keyList().contains("BackgroundNormal")
                && reversedGroup.keyList().contains("ForegroundNormal")) {
                //! reverse usual text/background values
                KConfigGroup normalGroup(normalPtr, groupName);

                reversedGroup.writeEntry("BackgroundNormal", normalGroup.readEntry("ForegroundNormal", QColor()));
                reversedGroup.writeEntry("ForegroundNormal", normalGroup.readEntry("BackgroundNormal", QColor()));

                reversedGroup.sync();
            }
        }

        //! update WM group
        KConfigGroup reversedGroup(reversedPtr, "WM");

        if (reversedGroup.keyList().contains("activeBackground")
            && reversedGroup.keyList().contains("activeForeground")
            && reversedGroup.keyList().contains("inactiveBackground")
            && reversedGroup.keyList().contains("inactiveForeground")) {
            //! reverse usual wm titlebar values
            KConfigGroup normalGroup(normalPtr, "WM");
            reversedGroup.writeEntry("activeBackground", normalGroup.readEntry("activeForeground", QColor()));
            reversedGroup.writeEntry("activeForeground", normalGroup.readEntry("activeBackground", QColor()));
            reversedGroup.writeEntry("inactiveBackground", normalGroup.readEntry("inactiveForeground", QColor()));
            reversedGroup.writeEntry("inactiveForeground", normalGroup.readEntry("inactiveBackground", QColor()));
            reversedGroup.sync();
        }

        if (reversedGroup.keyList().contains("activeBlend")
            && reversedGroup.keyList().contains("inactiveBlend")) {
            KConfigGroup normalGroup(normalPtr, "WM");
            reversedGroup.writeEntry("activeBlend", normalGroup.readEntry("inactiveBlend", QColor()));
            reversedGroup.writeEntry("inactiveBlend", normalGroup.readEntry("activeBlend", QColor()));
            reversedGroup.sync();
        }
    }
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

    foreach (auto key, roundGroup.keyList()) {
        if (m_theme.themeName().toUpper().startsWith(key.toUpper())) {
            QStringList rs = roundGroup.readEntry(key, QStringList());
            qDebug() << "roundness ::: " << rs;

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

void PlasmaThemeExtended::loadThemePaths()
{
    m_themePath = "";

    QString localD = QDir::homePath() + "/.local/share/plasma/desktoptheme/" + m_theme.themeName();
    QString globalD = "/usr/share/plasma/desktoptheme/" + m_theme.themeName();

    if (QDir(localD).exists()) {
        m_themePath = localD;
    } else if (QDir(globalD).exists()) {
        m_themePath = globalD;
    }

    qDebug() << "current plasma theme ::: " << m_theme.themeName();
    qDebug() << "theme path ::: " << m_themePath;

    //! clear kde connections
    for (auto &c : m_kdeConnections) {
        disconnect(c);
    }

    //! assign color schemes
    QString themeColorScheme = m_themePath + "/colors";

    if (QFileInfo(themeColorScheme).exists()) {
        setNormalSchemeFile(themeColorScheme);
    } else {
        //! when plasma theme uses the kde colors
        //! we track when kde color scheme is changing
        QString kdeSettingsFile = QDir::homePath() + "/.config/kdeglobals";

        KDirWatch::self()->addFile(kdeSettingsFile);

        m_kdeConnections[0] = connect(KDirWatch::self(), &KDirWatch::dirty, this, [ &, kdeSettingsFile](const QString & path) {
            if (path == kdeSettingsFile) {
                this->setNormalSchemeFile(SchemeColors::possibleSchemeFile("kdeglobals"));
            }
        });

        m_kdeConnections[1] = connect(KDirWatch::self(), &KDirWatch::created, this, [ &, kdeSettingsFile](const QString & path) {
            if (path == kdeSettingsFile) {
                this->setNormalSchemeFile(SchemeColors::possibleSchemeFile("kdeglobals"));
            }
        });

        setNormalSchemeFile(SchemeColors::possibleSchemeFile("kdeglobals"));
    }
}

void PlasmaThemeExtended::loadThemeLightness()
{
    float textColorLum = Latte::colorLumina(m_normalScheme->textColor());
    float backColorLum = Latte::colorLumina(m_normalScheme->backgroundColor());

    if (backColorLum > textColorLum) {
        m_isLightTheme = true;
    } else {
        m_isLightTheme = false;
    }

    if (m_isLightTheme) {
        qDebug() << "Plasma theme is light...";
    } else {
        qDebug() << "Plasma theme is dark...";
    }
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
