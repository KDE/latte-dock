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
#include "../../layouts/importer.h"
#include "../../view/panelshadows_p.h"
#include "../../wm/schemecolors.h"
#include "../../../liblatte2/commontools.h"

// Qt
#include <QDebug>
#include <QDir>
#include <QProcess>

// KDE
#include <KDirWatch>
#include <KConfigGroup>
#include <KSharedConfig>

// X11
#include <KWindowSystem>

#define DEFAULTCOLORSCHEME "default.colors"
#define REVERSEDCOLORSCHEME "reversed.colors"

namespace Latte {
namespace PlasmaExtended {

Theme::Theme(KSharedConfig::Ptr config, QObject *parent) :
    QObject(parent),
    m_themeGroup(KConfigGroup(config, QStringLiteral("PlasmaThemeExtended")))
{
    m_corona = qobject_cast<Latte::Corona *>(parent);

    //! compositing tracking
    if (KWindowSystem::isPlatformWayland()) {
        //! TODO: Wayland compositing active
        m_compositing = true;
    } else {
        connect(KWindowSystem::self(), &KWindowSystem::compositingChanged
                , this, [&](bool enabled) {
            if (m_compositing == enabled)
                return;

            m_compositing = enabled;
            emit compositingChanged();
        });

        m_compositing = KWindowSystem::compositingActive();
    }
    //!

    loadConfig();

    connect(this, &Theme::compositingChanged, this, &Theme::roundnessChanged);
    connect(this, &Theme::outlineWidthChanged, this, &Theme::saveConfig);

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
    return m_bottomEdgeRoundness;
}

int Theme::leftEdgeRoundness() const
{
    return m_leftEdgeRoundness;
}

int Theme::topEdgeRoundness() const
{
    return m_topEdgeRoundness;
}

int Theme::rightEdgeRoundness() const
{
    return m_rightEdgeRoundness;
}

int Theme::outlineWidth() const
{
    return m_outlineWidth;
}

void Theme::setOutlineWidth(int width)
{
    if (m_outlineWidth == width) {
        return;
    }

    m_outlineWidth = width;
    emit outlineWidthChanged();
}

float Theme::bottomEdgeMaxOpacity() const
{
    return m_bottomEdgeMaxOpacity;
}

float Theme::leftEdgeMaxOpacity() const
{
    return m_leftEdgeMaxOpacity;
}

float Theme::topEdgeMaxOpacity() const
{
    return m_topEdgeMaxOpacity;
}

float Theme::rightEdgeMaxOpacity() const
{
    return m_rightEdgeMaxOpacity;
}

WindowSystem::SchemeColors *Theme::defaultTheme() const
{
    return m_defaultScheme;
}

WindowSystem::SchemeColors *Theme::lightTheme() const
{
    return m_isLightTheme ? m_defaultScheme : m_reversedScheme;
}

WindowSystem::SchemeColors *Theme::darkTheme() const
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
        disconnect(m_defaultScheme, &WindowSystem::SchemeColors::colorsChanged, this, &Theme::loadThemeLightness);
        m_defaultScheme->deleteLater();
    }

    m_defaultScheme = new WindowSystem::SchemeColors(this, m_defaultSchemePath, true);
    connect(m_defaultScheme, &WindowSystem::SchemeColors::colorsChanged, this, &Theme::loadThemeLightness);

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

    m_reversedScheme = new WindowSystem::SchemeColors(this, m_reversedSchemePath, true);

    qDebug() << "plasma theme reversed colors ::: " << m_reversedSchemePath;
}

void Theme::updateReversedSchemeValues()
{
    //! reverse values based on original scheme
    KSharedConfigPtr originalPtr = KSharedConfig::openConfig(m_originalSchemePath);
    KSharedConfigPtr reversedPtr = KSharedConfig::openConfig(m_reversedSchemePath);

    if (originalPtr && reversedPtr) {
        for (const auto &groupName : reversedPtr->groupList()) {
            if (groupName != "Colors:Button" && groupName != "Colors:Selection") {
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
        QString originalSchemeName = WindowSystem::SchemeColors::schemeName(m_originalSchemePath);
        KConfigGroup generalGroup(reversedPtr, "General");
        generalGroup.writeEntry("Name", originalSchemeName + "_reversed");
        generalGroup.sync();
    }
}

int Theme::roundness(const QImage &svgImage, Plasma::Types::Location edge)
{
    int discovRow = (edge == Plasma::Types::TopEdge ? svgImage.height()-1 : 0);
    int discovCol = (edge == Plasma::Types::LeftEdge ? svgImage.width()-1 : 0);

    int round{0};

    int maxOpacity = qMin(qAlpha(svgImage.pixel(49,0)), 200);

    if (edge == Plasma::Types::BottomEdge) {
        m_bottomEdgeMaxOpacity = (float)maxOpacity / (float)255;
    } else if (edge == Plasma::Types::LeftEdge) {
        m_leftEdgeMaxOpacity = (float)maxOpacity / (float)255;
    } else if (edge == Plasma::Types::TopEdge) {
        m_topEdgeMaxOpacity = (float)maxOpacity / (float)255;
    } else if (edge == Plasma::Types::RightEdge) {
        m_rightEdgeMaxOpacity = (float)maxOpacity / (float)255;
    }

    if (edge == Plasma::Types::BottomEdge || edge == Plasma::Types::RightEdge || edge == Plasma::Types::TopEdge) {
        //! TOPLEFT corner
        //! first LEFT pixel found
        QRgb *line = (QRgb *)svgImage.scanLine(discovRow);

        for (int col=0; col<50; ++col) {
            QRgb pixelData = line[col];

            if (qAlpha(pixelData) < maxOpacity) {
                discovCol++;
                round++;
            } else {
                break;
            }
        }
    } else if (edge == Plasma::Types::LeftEdge) {
        //! it should be TOPRIGHT corner in that case
        //! first RIGHT pixel found
        QRgb *line = (QRgb *)svgImage.scanLine(discovRow);
        for (int col=99; col>50; --col) {
            QRgb pixelData = line[col];

            if (qAlpha(pixelData) < maxOpacity) {
                discovCol--;
                round++;
            } else {
                break;
            }
        }
    }

    //! this needs investigation (the x2) I don't know if it is really needed
    //! but it gives me the impression that returns better results
    return round; ///**2*/;
}

void Theme::loadCompositingRoundness()
{
    Plasma::FrameSvg *svg = new Plasma::FrameSvg(this);
    svg->setImagePath(QStringLiteral("widgets/panel-background"));
    svg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    svg->resizeFrame(QSize(100,100));

    //! New approach
    QPixmap pxm = svg->framePixmap();

    //! bottom roundness
    if (svg->hasElementPrefix("south")) {
        svg->setElementPrefix("south");
        pxm = svg->framePixmap();
    } else {
        svg->setElementPrefix("");
        pxm = svg->framePixmap();
    }
    m_bottomEdgeRoundness = roundness(pxm.toImage(), Plasma::Types::BottomEdge);

    //! left roundness
    if (svg->hasElementPrefix("west")) {
        svg->setElementPrefix("west");
        pxm = svg->framePixmap();
    } else {
        svg->setElementPrefix("");
        pxm = svg->framePixmap();
    }
    m_leftEdgeRoundness = roundness(pxm.toImage(), Plasma::Types::LeftEdge);

    //! top roundness
    if (svg->hasElementPrefix("north")) {
        svg->setElementPrefix("north");
        pxm = svg->framePixmap();
    } else {
        svg->setElementPrefix("");
        pxm = svg->framePixmap();
    }
    m_topEdgeRoundness = roundness(pxm.toImage(), Plasma::Types::TopEdge);

    //! right roundness
    if (svg->hasElementPrefix("east")) {
        svg->setElementPrefix("east");
        pxm = svg->framePixmap();
    } else {
        svg->setElementPrefix("");
        pxm = svg->framePixmap();
    }
    m_rightEdgeRoundness = roundness(pxm.toImage(), Plasma::Types::RightEdge);

  /*  qDebug() << " COMPOSITING MASK ::: " << svg->mask();
    qDebug() << " COMPOSITING MASK BOUNDING RECT ::: " << svg->mask().boundingRect();*/
    qDebug() << " COMPOSITING ROUNDNESS ::: " << m_bottomEdgeRoundness << " _ " << m_leftEdgeRoundness << " _ " << m_topEdgeRoundness << " _ " << m_rightEdgeRoundness;

    svg->deleteLater();
}

void Theme::loadRoundness()
{
    loadCompositingRoundness();

    emit maxOpacityChanged();
    emit roundnessChanged();
}

void Theme::loadThemePaths()
{
    m_themePath = Layouts::Importer::standardPath("plasma/desktoptheme/" + m_theme.themeName());

    if (QDir(m_themePath+"/widgets").exists()) {
        m_themeWidgetsPath = m_themePath + "/widgets";
    } else {
        m_themeWidgetsPath = Layouts::Importer::standardPath("plasma/desktoptheme/default/widgets");
    }

    qDebug() << "current plasma theme ::: " << m_theme.themeName();
    qDebug() << "theme path ::: " << m_themePath;
    qDebug() << "theme widgets path ::: " << m_themeWidgetsPath;

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
                this->setOriginalSchemeFile(WindowSystem::SchemeColors::possibleSchemeFile("kdeglobals"));
            }
        });

        m_kdeConnections[1] = connect(KDirWatch::self(), &KDirWatch::created, this, [ &, kdeSettingsFile](const QString & path) {
            if (path == kdeSettingsFile) {
                this->setOriginalSchemeFile(WindowSystem::SchemeColors::possibleSchemeFile("kdeglobals"));
            }
        });

        setOriginalSchemeFile(WindowSystem::SchemeColors::possibleSchemeFile("kdeglobals"));
    }

    //! this is probably not needed at all in order to provide full transparency for all
    //! plasma themes, so we disable it in order to confirm from user testing
    //! that it is not needed at all
    //parseThemeSvgFiles();
}

void Theme::parseThemeSvgFiles()
{
    QString origBackgroundSvgFile;
    QString curBackgroundSvgFile = m_extendedThemeDir.path()+"/widgets/panel-background.svg";

    if (QFileInfo(curBackgroundSvgFile).exists()) {
        QDir(m_extendedThemeDir.path()+"/widgets").remove("panel-background.svg");
    }

    if (!QDir(m_extendedThemeDir.path()+"/widgets").exists()) {
        QDir(m_extendedThemeDir.path()).mkdir("widgets");
    }

    if (QFileInfo(m_themeWidgetsPath+"/panel-background.svg").exists()) {
        origBackgroundSvgFile = m_themeWidgetsPath+"/panel-background.svg";
        QFile(origBackgroundSvgFile).copy(curBackgroundSvgFile);
    } else if (QFileInfo(m_themeWidgetsPath+"/panel-background.svgz").exists()) {
        origBackgroundSvgFile = m_themeWidgetsPath+"/panel-background.svgz";
        QString tempBackFile = m_extendedThemeDir.path()+"/widgets/panel-background.svg.gz";
        QFile(origBackgroundSvgFile).copy(tempBackFile);

        //! Identify Plasma Desktop version
        QProcess process;
        process.start("gzip -d " + tempBackFile);
        process.waitForFinished();
        QString output(process.readAllStandardOutput());

        qDebug() << "plasma theme, background extraction output ::: " << output;
        qDebug() << "plasma theme, original background svg file was decompressed...";
    }

    if (QFileInfo(curBackgroundSvgFile).exists()) {
        qDebug() << "plasma theme, panel background ::: " << curBackgroundSvgFile;
    } else {
        qDebug() << "plasma theme, panel background ::: was not found...";
    }

    //! Find panel-background transparency
    QFile svgFile(curBackgroundSvgFile);
    QString styleSvgStr;

    if (svgFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&svgFile);
        bool centerIdFound{false};
        bool styleFound{false};

        while (!in.atEnd() && !styleFound) {
            QString line = in.readLine();

            //! each time a rect starts then style can be reset
            if (line.contains("<rect")) {
                styleSvgStr = "";
            }

            //! identify the id "center
            if (line.contains("id=\"center\"")) {
                centerIdFound = true;
            }

            //! if valid style for center exists we can break
            if (centerIdFound && !styleSvgStr.isEmpty()) {
                break;
            }

            if (centerIdFound && line.contains("style=\"") ) {
                styleSvgStr = line;
            }

            //! when end of "center" you can break
            if (centerIdFound && line.contains("/rect>")) {
                break;
            }
        }
        svgFile.close();
    }

    if (!styleSvgStr.isEmpty()) {
        int styleInd = styleSvgStr.indexOf("style=");
        QString cleanedStr = styleSvgStr.remove(0, styleInd+7);
        int endInd = cleanedStr.indexOf("\"");
        styleSvgStr = cleanedStr.mid(0,endInd);

        QStringList styleValues = styleSvgStr.split(";");
        // qDebug() << "plasma theme, discovered svg style ::: " << styleValues;

        float opacity{1};
        float fillOpacity{1};

        for (QString &value : styleValues) {
            if (value.startsWith("opacity:")) {
                opacity = value.remove(0,8).toFloat();
            }
            if (value.startsWith("fill-opacity:")) {
                fillOpacity = value.remove(0,13).toFloat();
            }
        }

      //  m_backgroundMaxOpacity = opacity * fillOpacity;

      //  qDebug() << "plasma theme opacity :: " << m_backgroundMaxOpacity << " from : " << opacity << " * " << fillOpacity;
    }

 //   emit backgroundMaxOpacityChanged();
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
    setOutlineWidth(m_themeGroup.readEntry("outlineWidth", 1));
}

void Theme::saveConfig()
{
    m_themeGroup.writeEntry("outlineWidth", m_outlineWidth);

    m_themeGroup.sync();
}

}
}
