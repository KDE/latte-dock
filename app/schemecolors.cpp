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

#include "schemecolors.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include <KConfigGroup>
#include <KSharedConfig>

namespace Latte {

SchemeColors::SchemeColors(QObject *parent, QString scheme) :
    QObject(parent)
{
    QString pSchemeFile = possibleSchemeFile(scheme);

    if (QFileInfo(pSchemeFile).exists()) {
        m_schemeFile = pSchemeFile;
        m_schemeName = scheme;
    }

    updateScheme();
}

SchemeColors::~SchemeColors()
{
///
}

QColor SchemeColors::backgroundColor() const
{
    return subgroup() == Active ? m_activeBackgroundColor : m_inactiveBackgroundColor;
}

QColor SchemeColors::textColor() const
{
    return subgroup() == Active ? m_activeTextColor : m_inactiveTextColor;
}

QColor SchemeColors::highlightColor() const
{
    return m_highlightColor;
}

QColor SchemeColors::highlightedTextColor() const
{
    return m_highlightedTextColor;
}

QColor SchemeColors::positiveText() const
{
    return m_positiveColor;
}

QColor SchemeColors::neutralText() const
{
    return m_neutralText;
}

QColor SchemeColors::negativeText() const
{
    return m_negativeText;
}

QString SchemeColors::schemeName()
{
    return m_schemeName;
}

QString SchemeColors::schemeFile()
{
    return m_schemeFile;
}

SchemeColors::ColorsSubgroup SchemeColors::subgroup() const
{
    return m_subgroup;
}

void SchemeColors::setSubgroup(ColorsSubgroup subgroup)
{
    if (m_subgroup == subgroup) {
        return;
    }

    m_subgroup = subgroup;
    emit colorsChanged();
}


QString SchemeColors::possibleSchemeFile(QString scheme)
{
    if (scheme.startsWith("/") && scheme.endsWith(".colors") && QFileInfo(scheme).exists()) {
        return scheme;
    }

    QString tempScheme = scheme;

    if (scheme == "kdeglobals") {
        QString settingsFile = QDir::homePath() + "/.config/kdeglobals";

        if (QFileInfo(settingsFile).exists()) {
            KSharedConfigPtr filePtr = KSharedConfig::openConfig(settingsFile);
            KConfigGroup generalGroup = KConfigGroup(filePtr, "General");
            tempScheme = generalGroup.readEntry("ColorScheme", "");
        }
    }

    //! remove all whitespaces and "-" from scheme in order to access correctly its file
    QString schemeNameSimplified = tempScheme.simplified().remove(" ").remove("-");

    QString localSchemePath = QDir::homePath() + "/.local/share/color-schemes/" + schemeNameSimplified + ".colors";
    QString globalSchemePath = "/usr/share/color-schemes/" + schemeNameSimplified + ".colors";

    if (QFileInfo(localSchemePath).exists()) {
        return localSchemePath;
    } else if (QFileInfo(globalSchemePath).exists()) {
        return globalSchemePath;
    }

    return "";
}

void SchemeColors::updateScheme()
{
    if (m_schemeFile.isEmpty() || !QFileInfo(m_schemeFile).exists()) {
        return;
    }

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(m_schemeFile);
    KConfigGroup wmGroup = KConfigGroup(filePtr, "WM");
    KConfigGroup selGroup = KConfigGroup(filePtr, "Colors:Selection");
    KConfigGroup viewGroup = KConfigGroup(filePtr, "Colors:View");

    m_activeBackgroundColor = wmGroup.readEntry("activeBackground", QColor());
    m_activeTextColor = wmGroup.readEntry("activeForeground", QColor());

    m_inactiveBackgroundColor = wmGroup.readEntry("inactiveBackground", QColor());
    m_inactiveTextColor = wmGroup.readEntry("inactiveForeground", QColor());

    m_highlightColor = selGroup.readEntry("BackgroundNormal", QColor());
    m_highlightedTextColor = selGroup.readEntry("ForegroundNormal", QColor());

    m_positiveColor = selGroup.readEntry("ForegroundPositive", QColor());
    m_neutralText = selGroup.readEntry("ForegroundNeutral", QColor());;
    m_negativeText = selGroup.readEntry("ForegroundNegative", QColor());

    emit colorsChanged();
}

}
