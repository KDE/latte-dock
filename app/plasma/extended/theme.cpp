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

#include "theme.h"

// local
#include "lattecorona.h"
#include "schemecolors.h"
#include "../../view/panelshadows_p.h"
#include "../../../liblatte2/commontools.h"

// Qt
#include <QDebug>
#include <QDir>

// KDE
#include <KDirWatch>
#include <KConfigGroup>
#include <KSharedConfig>

#define DEFAULTCOLORSCHEME "default.colors"
#define REVERSEDCOLORSCHEME "reversed.colors"

namespace Latte {
namespace PlasmaExtended {

Theme::Theme(KSharedConfig::Ptr config, QObject *parent) :
    QObject(parent),
    m_themeGroup(KConfigGroup(config, QStringLiteral("PlasmaThemeExtended")))
{
    m_corona = qobject_cast<Latte::Corona *>(parent);

    loadConfig();

    connect(&m_theme, &Plasma::Theme::themeChanged, this, &Theme::hasShadowChanged);
    connect(&m_theme, &Plasma::Theme::themeChanged, this, &Theme::load);
    connect(&m_theme, &Plasma::Theme::themeChanged, this, &Theme::themeChanged);
}

void Theme::load()
{
    loadThemePaths();
    loadRoundness();
}

Theme::~Theme()
{
    saveConfig();

    m_defaultScheme->deleteLater();
    m_reversedScheme->deleteLater();
}

bool Theme::hasShadow() const
{
    return PanelShadows::self()->enabled();
}

bool Theme::isLightTheme() const
{
    return m_isLightTheme;
}

bool Theme::isDarkTheme() const
{
    return !m_isLightTheme;
}

int Theme::bottomEdgeRoundness() const
{
    return ((themeHasExtendedInfo() && m_userRoundness == -1) ? m_bottomEdgeRoundness : userThemeRoundness());
}

int Theme::leftEdgeRoundness() const
{
    return ((themeHasExtendedInfo() && m_userRoundness == -1) ? m_leftEdgeRoundness : userThemeRoundness());
}

int Theme::topEdgeRoundness() const
{
    return ((themeHasExtendedInfo() && m_userRoundness == -1) ? m_topEdgeRoundness : userThemeRoundness());
}

int Theme::rightEdgeRoundness() const
{
    return ((themeHasExtendedInfo() && m_userRoundness == -1) ? m_rightEdgeRoundness : userThemeRoundness());
}

int Theme::userThemeRoundness() const
{
    return qMax(0, m_userRoundness);
}

void Theme::setUserThemeRoundness(int roundness)
{
    if (m_userRoundness == roundness) {
        return;
    }

    m_userRoundness = roundness;

    emit roundnessChanged();

    saveConfig();
}

bool Theme::themeHasExtendedInfo() const
{
    return m_themeHasExtendedInfo;
}

SchemeColors *Theme::defaultTheme() const
{
    return m_defaultScheme;
}

SchemeColors *Theme::lightTheme() const
{
    return m_isLightTheme ? m_defaultScheme : m_reversedScheme;
}

SchemeColors *Theme::darkTheme() const
{
    return !m_isLightTheme ? m_defaultScheme : m_reversedScheme;
}


void Theme::setOriginalSchemeFile(const QString &file)
{
    if (m_originalSchemePath == file) {
        return;
    }

    m_originalSchemePath = file;

    qDebug() << "plasma theme original colors ::: " << m_originalSchemePath;

    updateDefaultScheme();
    updateReversedScheme();

    loadThemeLightness();

    emit themeChanged();
}

//! WM records need to be updated based on the colors that
//! plasma will use in order to be consistent. Such an example
//! are the Breeze color schemes that have different values for
//! WM and the plasma theme records
void Theme::updateDefaultScheme()
{
    QString defaultFilePath = m_extendedThemeDir.path() + "/" + DEFAULTCOLORSCHEME;
    if (QFileInfo(defaultFilePath).exists()) {
        QFile(defaultFilePath).remove();
    }

    QFile(m_originalSchemePath).copy(defaultFilePath);
    m_defaultSchemePath = defaultFilePath;

    updateDefaultSchemeValues();

    if (m_defaultScheme) {
        disconnect(m_defaultScheme, &SchemeColors::colorsChanged, this, &Theme::loadThemeLightness);
        m_defaultScheme->deleteLater();
    }

    m_defaultScheme = new SchemeColors(this, m_defaultSchemePath, true);
    connect(m_defaultScheme, &SchemeColors::colorsChanged, this, &Theme::loadThemeLightness);

    qDebug() << "plasma theme default colors ::: " << m_defaultSchemePath;
}

void Theme::updateDefaultSchemeValues()
{
    //! update WM values based on original scheme
    KSharedConfigPtr originalPtr = KSharedConfig::openConfig(m_originalSchemePath);
    KSharedConfigPtr defaultPtr = KSharedConfig::openConfig(m_defaultSchemePath);

    if (originalPtr && defaultPtr) {
        KConfigGroup originalViewGroup(originalPtr, "Colors:View");
        KConfigGroup defaultWMGroup(defaultPtr, "WM");

        defaultWMGroup.writeEntry("activeBackground", originalViewGroup.readEntry("BackgroundNormal", QColor()));
        defaultWMGroup.writeEntry("activeForeground", originalViewGroup.readEntry("ForegroundNormal", QColor()));

        defaultWMGroup.sync();
    }
}

void Theme::updateReversedScheme()
{
    QString reversedFilePath = m_extendedThemeDir.path() + "/" + REVERSEDCOLORSCHEME;

    if (QFileInfo(reversedFilePath).exists()) {
        QFile(reversedFilePath).remove();
    }

    QFile(m_originalSchemePath).copy(reversedFilePath);
    m_reversedSchemePath = reversedFilePath;

    updateReversedSchemeValues();

    if (m_reversedScheme) {
        m_reversedScheme->deleteLater();
    }

    m_reversedScheme = new SchemeColors(this, m_reversedSchemePath, true);

    qDebug() << "plasma theme reversed colors ::: " << m_reversedSchemePath;
}

void Theme::updateReversedSchemeValues()
{
    //! reverse values based on original scheme
    KSharedConfigPtr originalPtr = KSharedConfig::openConfig(m_originalSchemePath);
    KSharedConfigPtr reversedPtr = KSharedConfig::openConfig(m_reversedSchemePath);

    if (originalPtr && reversedPtr) {
        foreach (auto groupName, reversedPtr->groupList()) {
            if (groupName != "Colors:Button") {
                KConfigGroup reversedGroup(reversedPtr, groupName);

                if (reversedGroup.keyList().contains("BackgroundNormal")
                    && reversedGroup.keyList().contains("ForegroundNormal")) {
                    //! reverse usual text/background values
                    KConfigGroup originalGroup(originalPtr, groupName);

                    reversedGroup.writeEntry("BackgroundNormal", originalGroup.readEntry("ForegroundNormal", QColor()));
                    reversedGroup.writeEntry("ForegroundNormal", originalGroup.readEntry("BackgroundNormal", QColor()));

                    reversedGroup.sync();
                }
            }
        }

        //! update WM group
        KConfigGroup reversedWMGroup(reversedPtr, "WM");
        KConfigGroup originalViewGroup(originalPtr, "Colors:View");

        if (reversedWMGroup.keyList().contains("activeBackground")
            && reversedWMGroup.keyList().contains("activeForeground")
            && reversedWMGroup.keyList().contains("inactiveBackground")
            && reversedWMGroup.keyList().contains("inactiveForeground")) {
            //! reverse usual wm titlebar values
            KConfigGroup originalGroup(originalPtr, "WM");
            reversedWMGroup.writeEntry("activeBackground", originalViewGroup.readEntry("ForegroundNormal", QColor()));
            reversedWMGroup.writeEntry("activeForeground", originalViewGroup.readEntry("BackgroundNormal", QColor()));
            reversedWMGroup.writeEntry("inactiveBackground", originalGroup.readEntry("inactiveForeground", QColor()));
            reversedWMGroup.writeEntry("inactiveForeground", originalGroup.readEntry("inactiveBackground", QColor()));
            reversedWMGroup.sync();
        }

        if (reversedWMGroup.keyList().contains("activeBlend")
            && reversedWMGroup.keyList().contains("inactiveBlend")) {
            KConfigGroup originalGroup(originalPtr, "WM");
            reversedWMGroup.writeEntry("activeBlend", originalGroup.readEntry("inactiveBlend", QColor()));
            reversedWMGroup.writeEntry("inactiveBlend", originalGroup.readEntry("activeBlend", QColor()));
            reversedWMGroup.sync();
        }

        //! update scheme name
        QString originalSchemeName = SchemeColors::schemeName(m_originalSchemePath);
        KConfigGroup generalGroup(reversedPtr, "General");
        generalGroup.writeEntry("Name", originalSchemeName + "_reversed");
        generalGroup.sync();
    }
}


void Theme::loadRoundness()
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

void Theme::loadThemePaths()
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
        setOriginalSchemeFile(themeColorScheme);
    } else {
        //! when plasma theme uses the kde colors
        //! we track when kde color scheme is changing
        QString kdeSettingsFile = QDir::homePath() + "/.config/kdeglobals";

        KDirWatch::self()->addFile(kdeSettingsFile);

        m_kdeConnections[0] = connect(KDirWatch::self(), &KDirWatch::dirty, this, [ &, kdeSettingsFile](const QString & path) {
            if (path == kdeSettingsFile) {
                this->setOriginalSchemeFile(SchemeColors::possibleSchemeFile("kdeglobals"));
            }
        });

        m_kdeConnections[1] = connect(KDirWatch::self(), &KDirWatch::created, this, [ &, kdeSettingsFile](const QString & path) {
            if (path == kdeSettingsFile) {
                this->setOriginalSchemeFile(SchemeColors::possibleSchemeFile("kdeglobals"));
            }
        });

        setOriginalSchemeFile(SchemeColors::possibleSchemeFile("kdeglobals"));
    }
}

void Theme::loadThemeLightness()
{
    float textColorLum = Latte::colorLumina(m_defaultScheme->textColor());
    float backColorLum = Latte::colorLumina(m_defaultScheme->backgroundColor());

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

void Theme::loadConfig()
{
    setUserThemeRoundness(m_themeGroup.readEntry("userSetPlasmaThemeRoundness", -1));
}

void Theme::saveConfig()
{
    m_themeGroup.writeEntry("userSetPlasmaThemeRoundness", m_userRoundness);

    m_themeGroup.sync();
}

}
}
