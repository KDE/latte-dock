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

// local
#include "../layouts/importer.h"

// Qt
#include <QDebug>
#include <QDir>
#include <QFileInfo>

// KDE
#include <KConfigGroup>
#include <KDirWatch>
#include <KSharedConfig>

namespace Latte {
namespace WindowSystem {

SchemeColors::SchemeColors(QObject *parent, QString scheme, bool plasmaTheme) :
    QObject(parent),
    m_basedOnPlasmaTheme(plasmaTheme)
{
    QString pSchemeFile = possibleSchemeFile(scheme);

    if (QFileInfo(pSchemeFile).exists()) {
        setSchemeFile(pSchemeFile);
        m_schemeName = schemeName(pSchemeFile);

        //! track scheme file for changes
        KDirWatch::self()->addFile(m_schemeFile);

        connect(KDirWatch::self(), &KDirWatch::dirty, this, [ & ](const QString & path) {
            if (path == m_schemeFile) {
                updateScheme();
            }
        });
    }

    updateScheme();
}

SchemeColors::~SchemeColors()
{
    ///
}

QColor SchemeColors::backgroundColor() const
{
    return m_activeBackgroundColor;
}

QColor SchemeColors::textColor() const
{
    return m_activeTextColor;
}

QColor SchemeColors::inactiveBackgroundColor() const
{
    return m_inactiveBackgroundColor;
}

QColor SchemeColors::inactiveTextColor() const
{
    return m_inactiveTextColor;
}

QColor SchemeColors::highlightColor() const
{
    return m_highlightColor;
}

QColor SchemeColors::highlightedTextColor() const
{
    return m_highlightedTextColor;
}

QColor SchemeColors::positiveTextColor() const
{
    return m_positiveTextColor;
}

QColor SchemeColors::neutralTextColor() const
{
    return m_neutralTextColor;
}

QColor SchemeColors::negativeTextColor() const
{
    return m_negativeTextColor;
}

QColor SchemeColors::buttonTextColor() const
{
    return m_buttonTextColor;
}

QColor SchemeColors::buttonBackgroundColor() const
{
    return m_buttonBackgroundColor;
}

QColor SchemeColors::buttonHoverColor() const
{
    return m_buttonHoverColor;
}

QColor SchemeColors::buttonFocusColor() const
{
    return m_buttonFocusColor;
}

QString SchemeColors::schemeName() const
{
    return m_schemeName;
}

QString SchemeColors::SchemeColors::schemeFile() const
{
    return m_schemeFile;
}

void SchemeColors::setSchemeFile(QString file)
{
    if (m_schemeFile == file) {
        return;
    }

    m_schemeFile = file;
    emit schemeFileChanged();
}

QString SchemeColors::possibleSchemeFile(QString scheme)
{
    if (scheme.startsWith("/") && scheme.endsWith("colors") && QFileInfo(scheme).exists()) {
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

    QString schemePath = Layouts::Importer::standardPath("color-schemes/" + tempScheme + ".colors");

    if (schemePath.isEmpty() || !QFileInfo(schemePath).exists()) {
        //! remove all whitespaces and "-" from scheme in order to access correctly its file
        QString schemeNameSimplified = tempScheme.simplified().remove(" ").remove("-");

        schemePath = Layouts::Importer::standardPath("color-schemes/" + schemeNameSimplified + ".colors");
    }

    if (QFileInfo(schemePath).exists()) {
        return schemePath;
    }

    return "";
}

QString SchemeColors::schemeName(QString originalFile)
{
    if (!(originalFile.startsWith("/") && originalFile.endsWith("colors") && QFileInfo(originalFile).exists())) {
        return "";
    }

    QString fileNameNoExt =  originalFile;

    int lastSlash = originalFile.lastIndexOf("/");

    if (lastSlash >= 0) {
        fileNameNoExt.remove(0, lastSlash + 1);
    }

    if (fileNameNoExt.endsWith(".colors")) {
        fileNameNoExt.remove(".colors");
    }

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(originalFile);
    KConfigGroup generalGroup = KConfigGroup(filePtr, "General");

    return generalGroup.readEntry("Name", fileNameNoExt);
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
    //KConfigGroup windowGroup = KConfigGroup(filePtr, "Colors:Window");
    KConfigGroup buttonGroup = KConfigGroup(filePtr, "Colors:Button");

    if (!m_basedOnPlasmaTheme) {
        m_activeBackgroundColor = wmGroup.readEntry("activeBackground", QColor());
        m_activeTextColor = wmGroup.readEntry("activeForeground", QColor());
        m_inactiveBackgroundColor = wmGroup.readEntry("inactiveBackground", QColor());
        m_inactiveTextColor = wmGroup.readEntry("inactiveForeground", QColor());
    } else {
        m_activeBackgroundColor = viewGroup.readEntry("BackgroundNormal", QColor());
        m_activeTextColor = viewGroup.readEntry("ForegroundNormal", QColor());
        m_inactiveBackgroundColor = viewGroup.readEntry("BackgroundAlternate", QColor());
        m_inactiveTextColor = viewGroup.readEntry("ForegroundInactive", QColor());
    }

    m_highlightColor = selGroup.readEntry("BackgroundNormal", QColor());
    m_highlightedTextColor = selGroup.readEntry("ForegroundNormal", QColor());

    m_positiveTextColor = viewGroup.readEntry("ForegroundPositive", QColor());
    m_neutralTextColor = viewGroup.readEntry("ForegroundNeutral", QColor());;
    m_negativeTextColor = viewGroup.readEntry("ForegroundNegative", QColor());

    m_buttonTextColor = buttonGroup.readEntry("ForegroundNormal", QColor());
    m_buttonBackgroundColor = buttonGroup.readEntry("BackgroundNormal", QColor());
    m_buttonHoverColor = buttonGroup.readEntry("DecorationHover", QColor());
    m_buttonFocusColor = buttonGroup.readEntry("DecorationFocus", QColor());

    emit colorsChanged();
}

}
}
