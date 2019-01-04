/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "backgroundcache.h"

// local
#include "commontools.h"

// Qt
#include <QDebug>
#include <QImage>
#include <QRgb>
#include <QtMath>

// Plasma
#include <Plasma>

// KDE
#include <KConfigGroup>
#include <KDirWatch>

#define PLASMACONFIG "plasma-org.kde.plasma.desktop-appletsrc"
#define DEFAULTWALLPAPER "/usr/share/wallpapers/Next/contents/images/1920x1080.png"

namespace Latte{
namespace PlasmaExtended {

BackgroundCache::BackgroundCache(QObject *parent)
    : QObject(parent),
      m_initialized(false),
      m_plasmaConfig(KSharedConfig::openConfig(PLASMACONFIG))
{
    const auto configFile = QStandardPaths::writableLocation(
                QStandardPaths::GenericConfigLocation) +
            QLatin1Char('/') + PLASMACONFIG;

    KDirWatch::self()->addFile(configFile);

    connect(KDirWatch::self(), &KDirWatch::dirty, this, &BackgroundCache::settingsFileChanged);
    connect(KDirWatch::self(), &KDirWatch::created, this, &BackgroundCache::settingsFileChanged);

    if (!m_pool) {
        m_pool = new ScreenPool(this);
    }

    reload();
}

BackgroundCache::~BackgroundCache()
{
    if (m_pool) {
        m_pool->deleteLater();
    }
}

BackgroundCache *BackgroundCache::self()
{
    static BackgroundCache cache;
    return &cache;
}

void BackgroundCache::settingsFileChanged(const QString &file) {
    if (!file.endsWith(PLASMACONFIG)) {
        return;
    }

    if (m_initialized) {
        m_plasmaConfig->reparseConfiguration();
        reload();
    }
}

QString BackgroundCache::backgroundFromConfig(const KConfigGroup &config) const {
    auto wallpaperPlugin = config.readEntry("wallpaperplugin");
    auto wallpaperConfig = config.group("Wallpaper").group(wallpaperPlugin).group("General");

    if (wallpaperConfig.hasKey("Image")) {
        // Trying for the wallpaper
        auto wallpaper = wallpaperConfig.readEntry("Image", QString());

        if (!wallpaper.isEmpty()) {
            return wallpaper;
        }
    }

    if (wallpaperConfig.hasKey("Color")) {
        auto backgroundColor = wallpaperConfig.readEntry("Color", QColor(0, 0, 0));
        return backgroundColor.name();
    }

    return QString();
}

bool BackgroundCache::isDesktopContainment(const KConfigGroup &containment) const
{
    const auto type = containment.readEntry("plugin", QString());

    if (type == "org.kde.desktopcontainment" || type == "org.kde.plasma.folder" ) {
        return true;
    }

    return false;
}

void BackgroundCache::reload()
{
    // Traversing through all containments in search for
    // containments that define activities in plasma
    KConfigGroup plasmaConfigContainments = m_plasmaConfig->group("Containments");

    //!activityId and screen names for which their background was updated
    QHash<QString, QList<QString>> updates;

    for (const auto &containmentId : plasmaConfigContainments.groupList()) {
        const auto containment = plasmaConfigContainments.group(containmentId);
        const auto lastScreen  = containment.readEntry("lastScreen", 0);
        const auto activity    = containment.readEntry("activityId", QString());

        //! Ignore the containment if the activity is not defined or
        //! the containment is not a plasma desktop
        if (activity.isEmpty() || !isDesktopContainment(containment)) continue;

        const auto returnedBackground = backgroundFromConfig(containment);

        QString background = returnedBackground;

        if (background.startsWith("file://")) {
            background = returnedBackground.mid(7);
        }

        if (background.isEmpty()) continue;

        QString screenName = m_pool->connector(lastScreen);

        if(!m_backgrounds.contains(activity)
                || !m_backgrounds[activity].contains(screenName)
                || m_backgrounds[activity][screenName] != background) {

            updates[activity].append(screenName);
        }

        m_backgrounds[activity][screenName] = background;
    }

    m_initialized = true;

    foreach (auto activity, updates.keys()) {
        foreach (auto screen, updates[activity]) {
            emit backgroundChanged(activity, screen);
        }
    }
}

QString BackgroundCache::background(QString activity, QString screen)
{
    if (m_backgrounds.contains(activity) && m_backgrounds[activity].contains(screen)) {
        return m_backgrounds[activity][screen];
    } else {
        return DEFAULTWALLPAPER;
    }
}

bool BackgroundCache::distortedFor(QString activity, QString screen, Plasma::Types::Location location)
{
    QString assignedBackground = background(activity, screen);

    if (!assignedBackground.isEmpty()) {
        return distortedForFile(assignedBackground, location);
    }

    return false;
}

float BackgroundCache::brightnessFor(QString activity, QString screen, Plasma::Types::Location location)
{
    QString assignedBackground = background(activity, screen);

    if (!assignedBackground.isEmpty()) {
        return brightnessForFile(assignedBackground, location);
    }

    return -1000;
}

float BackgroundCache::brightnessFromArea(QImage &image, int firstRow, int firstColumn, int endRow, int endColumn)
{
    float areaBrightness = -1000;

    if (image.format() != QImage::Format_Invalid) {
        for (int row = firstRow; row < endRow; ++row) {
            QRgb *line = (QRgb *)image.scanLine(row);

            for (int col = firstColumn; col < endColumn ; ++col) {
                QRgb pixelData = line[col];
                float pixelBrightness = Latte::colorBrightness(pixelData);

                areaBrightness = (areaBrightness == -1000) ? pixelBrightness : (areaBrightness + pixelBrightness);
            }
        }

        float areaSize = (endRow - firstRow) * (endColumn - firstColumn);
        areaBrightness = areaBrightness / areaSize;
    }

    return areaBrightness;
}

bool BackgroundCache::areaIsDistorted(float bright1, float bright2, float bright3)
{
    int distortedStep{30};

    return (qFabs(bright1-bright2)>=distortedStep
            || qFabs(bright2-bright3)>=distortedStep
            || qFabs(bright1-bright3)>=distortedStep);
}

void BackgroundCache::updateImageCalculations(QString imageFile, Plasma::Types::Location location)
{
    //! if it is a local image
    QImage image(imageFile);

    if (image.format() != QImage::Format_Invalid) {
        int maskHeight = (0.08 * image.height());
        int maskWidth = (0.05 * image.width());

        float areaBrightness = -1000;

        float area1Brightness = -1000; float area2Brightness = -1000; float area3Brightness = -1000;
        float area1p = 0.25; float area2p=0.35; float area3p=0.4;

        int firstRow = 0; int firstColumn = 0; int endRow = 0; int endColumn = 0;

        //! horizontal mask calculations
        if (location == Plasma::Types::TopEdge) {
            firstRow = 0; endRow = maskHeight;
        } else if (location == Plasma::Types::BottomEdge) {
            firstRow = image.height() - maskHeight - 1; endRow = image.height() - 1;
        }

        if (location == Plasma::Types::TopEdge || location == Plasma::Types::BottomEdge) {
            firstColumn = 0; endColumn = (area1p*image.width()) - 1;
            area1Brightness =  brightnessFromArea(image, firstRow, firstColumn, endRow, endColumn);

            firstColumn = endColumn+1; endColumn = ((area1p+area2p)*image.width()) - 1;
            area2Brightness =  brightnessFromArea(image, firstRow, firstColumn, endRow, endColumn);

            firstColumn = endColumn+1; endColumn = (image.width() - 1);
            area3Brightness =  brightnessFromArea(image, firstRow, firstColumn, endRow, endColumn);
        }

        //! vertical mask calculations
        if (location == Plasma::Types::LeftEdge) {
            firstColumn = 0; endColumn = maskWidth;
        } else if (location == Plasma::Types::RightEdge) {
            firstColumn = image.width() - 1 - maskWidth; endColumn = image.width() - 1;
        }

        if (location == Plasma::Types::LeftEdge || location == Plasma::Types::RightEdge) {
            firstRow = 0; endRow = image.height() - 1;
            area1Brightness =  brightnessFromArea(image, firstRow, firstColumn, endRow, endColumn);

            firstRow = endRow+1; endRow = ((area1p+area2p)*image.height()) - 1;
            area2Brightness =  brightnessFromArea(image, firstRow, firstColumn, endRow, endColumn);

            firstRow = endRow+1; endRow = (image.height() - 1);
            area3Brightness =  brightnessFromArea(image, firstRow, firstColumn, endRow, endColumn);
        }

        //! compute total brightness for this area
        areaBrightness = (area1Brightness + area2Brightness + area3Brightness) / 3;
        bool areaDistorted = areaIsDistorted(area1Brightness, area2Brightness, area3Brightness);

        if (!m_hintsCache.keys().contains(imageFile)) {
            m_hintsCache[imageFile] = EdgesHash();
        }

        if (!m_hintsCache[imageFile].contains(location)) {
            imageHints iHints;
            iHints.brightness = areaBrightness;
            iHints.distorted =areaDistorted;
            m_hintsCache[imageFile].insert(location, iHints);
        } else {
            m_hintsCache[imageFile][location].brightness = areaBrightness;
            m_hintsCache[imageFile][location].distorted = areaDistorted;
        }
    }
}

float BackgroundCache::brightnessForFile(QString imageFile, Plasma::Types::Location location)
{
    if (m_hintsCache.keys().contains(imageFile)) {
        if (m_hintsCache[imageFile].keys().contains(location)) {
            return m_hintsCache[imageFile][location].brightness;
        }
    }

    //! if it is a color
    if (imageFile.startsWith("#")) {
        return Latte::colorBrightness(QColor(imageFile));
    }

    updateImageCalculations(imageFile, location);

    if (m_hintsCache.keys().contains(imageFile)) {
        return m_hintsCache[imageFile][location].brightness;
    }

    return -1000;
}

bool BackgroundCache::distortedForFile(QString imageFile, Plasma::Types::Location location)
{
    if (m_hintsCache.keys().contains(imageFile)) {
        if (m_hintsCache[imageFile].keys().contains(location)) {
            return m_hintsCache[imageFile][location].distorted;
        }
    }

    //! if it is a color
    if (imageFile.startsWith("#")) {
        return false;
    }

    updateImageCalculations(imageFile, location);

    if (m_hintsCache.keys().contains(imageFile)) {
        return m_hintsCache[imageFile][location].distorted;
    }

    return false;
}

}
}
