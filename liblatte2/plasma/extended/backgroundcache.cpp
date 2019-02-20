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
#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QRgb>
#include <QtMath>

// Plasma
#include <Plasma>

// KDE
#include <KConfigGroup>
#include <KDirWatch>

#define MAXHASHSIZE 300

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

QString BackgroundCache::backgroundFromConfig(const KConfigGroup &config, QString wallpaperPlugin) const {
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
        const auto wallpaperPlugin = containment.readEntry("wallpaperplugin", QString());
        const auto lastScreen  = containment.readEntry("lastScreen", 0);
        const auto activity    = containment.readEntry("activityId", QString());

        //! Ignore the containment if the activity is not defined or
        //! the containment is not a plasma desktop
        if (activity.isEmpty() || !isDesktopContainment(containment)) continue;

        const auto returnedBackground = backgroundFromConfig(containment, wallpaperPlugin);

        QString background = returnedBackground;

        if (background.startsWith("file://")) {
            background = returnedBackground.mid(7);
        }

        QString screenName = m_pool->connector(lastScreen);

        //! Take case of broadcasted backgrounds, when their plugin is changed they should be disabled
        if (pluginExistsFor(activity,screenName)
                && m_plugins[activity][screenName] != wallpaperPlugin
                && backgroundIsBroadcasted(activity, screenName)){
            //! in such case the Desktop changed wallpaper plugin and the broadcasted wallpapers should be removed
            setBroadcastedBackgroundsEnabled(activity, screenName, false);
        }

        m_plugins[activity][screenName] = wallpaperPlugin;

        if (background.isEmpty() || backgroundIsBroadcasted(activity, screenName)) {
            continue;
        }

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

bool BackgroundCache::busyFor(QString activity, QString screen, Plasma::Types::Location location)
{
    QString assignedBackground = background(activity, screen);

    if (!assignedBackground.isEmpty()) {
        return busyForFile(assignedBackground, location);
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

bool BackgroundCache::areaIsBusy(float bright1, float bright2)
{
    bool bright1IsLight = bright1>=123;
    bool bright2IsLight = bright2>=123;

    bool inBounds = bright1>=0 && bright2<=255 && bright2>=0 && bright2<=255;

    return !inBounds || bright1IsLight != bright2IsLight;
}

//! In order to calculate the brightness and busy hints for specific image
//! the code is doing the following. It is not needed to calculate these values
//! for the entire image that would also be cpu costly. The function takes
//! the location of the area in the image for which we are interested.
//! The area is splitted in ten different subareas and for each one its brightness
//! is computed. The brightness average from these areas provides the entire
//! area brightness. In order to indicate if this area is busy or not we
//! compare the minimum and the maximum values of brightness from these
//! subareas. If the difference it too big then the area is busy
void BackgroundCache::updateImageCalculations(QString imageFile, Plasma::Types::Location location)
{
    if (m_hintsCache.size() > MAXHASHSIZE) {
        cleanupHashes();
    }

    //! if it is a local image
    QImage image(imageFile);

    if (image.format() != QImage::Format_Invalid) {
        float brightness{-1000};
        float maxBrightness{0};
        float minBrightness{255};

        //! 24px. should be enough because the views are always snapped to edges
        int maskHeight = qMin(24,image.height()); // (0.08 * image.height());
        int maskWidth = qMin(24,image.width()); //(0.05 * image.width());

        bool vertical = image.width() > image.height() ? false : true;
        int imageLength = image.width() > image.height() ? image.width() : image.height();
        int areas{qMin(10,imageLength)};

        float factor = ((float)100/areas)/100;

        QList<float> subBrightness;

        //! Iterating algorigthm
        int firstRow = 0; int firstColumn = 0; int endRow = 0; int endColumn = 0;

        //! horizontal mask calculations
        if (location == Plasma::Types::TopEdge) {
            firstRow = 0; endRow = maskHeight;
        } else if (location == Plasma::Types::BottomEdge) {
            firstRow = image.height() - maskHeight - 1; endRow = image.height() - 1;
        }

        if (!vertical) {
            for (int i=1; i<=areas; ++i) {
                float subFactor = ((float)i) * factor;
                firstColumn = endColumn+1; endColumn = (subFactor*imageLength) - 1;
                endColumn = qMin(endColumn, imageLength-1);

                int tempBrightness = brightnessFromArea(image, firstRow, firstColumn, endRow, endColumn);
                subBrightness.append(tempBrightness);

                if (tempBrightness > maxBrightness) {
                    maxBrightness = tempBrightness;
                }
                if (tempBrightness < minBrightness) {
                    minBrightness = tempBrightness;
                }
            }
        }

        //! vertical mask calculations
        if (location == Plasma::Types::LeftEdge) {
            firstColumn = 0; endColumn = maskWidth;
        } else if (location == Plasma::Types::RightEdge) {
            firstColumn = image.width() - 1 - maskWidth; endColumn = image.width() - 1;
        }

        if (vertical) {
            for (int i=1; i<=areas; ++i) {
                float subFactor = ((float)i) * factor;
                firstRow = endRow+1; endRow = (subFactor*imageLength) - 1;
                endRow = qMin(endRow, imageLength-1);

                int tempBrightness = brightnessFromArea(image, firstRow, firstColumn, endRow, endColumn);
                subBrightness.append(tempBrightness);

                if (tempBrightness > maxBrightness) {
                    maxBrightness = tempBrightness;
                }
                if (tempBrightness < minBrightness) {
                    minBrightness = tempBrightness;
                }
            }
        }
        //! compute total brightness for this area
        float subBrightnessSum = 0;

        for (int i=0; i<subBrightness.count(); ++i) {
            subBrightnessSum = subBrightnessSum + subBrightness[i];
        }

        brightness = subBrightnessSum / subBrightness.count();

        bool areaBusy = areaIsBusy(minBrightness, maxBrightness);

        qDebug() << " Hints for Background image: " << imageFile;
        qDebug() << " Brightness: " << brightness << " Busy: " << areaBusy << " minBright:" << minBrightness << " maxBright:" << maxBrightness;

        if (!m_hintsCache.keys().contains(imageFile)) {
            m_hintsCache[imageFile] = EdgesHash();
        }

        if (!m_hintsCache[imageFile].contains(location)) {
            imageHints iHints;
            iHints.brightness = brightness; iHints.busy = areaBusy;
            m_hintsCache[imageFile].insert(location, iHints);
        } else {
            m_hintsCache[imageFile][location].brightness = brightness;
            m_hintsCache[imageFile][location].busy = areaBusy;
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

bool BackgroundCache::busyForFile(QString imageFile, Plasma::Types::Location location)
{
    if (m_hintsCache.keys().contains(imageFile)) {
        if (m_hintsCache[imageFile].keys().contains(location)) {
            return m_hintsCache[imageFile][location].busy;
        }
    }

    //! if it is a color
    if (imageFile.startsWith("#")) {
        return false;
    }

    updateImageCalculations(imageFile, location);

    if (m_hintsCache.keys().contains(imageFile)) {
        return m_hintsCache[imageFile][location].busy;
    }

    return false;
}

void BackgroundCache::cleanupHashes()
{
    if (m_hintsCache.count() <= MAXHASHSIZE) {
        return;
    }

    m_hintsCache.clear();
}

void BackgroundCache::setBackgroundFromBroadcast(QString activity, QString screen, QString filename)
{
    if (QFileInfo(filename).exists()) {
        setBroadcastedBackgroundsEnabled(activity, screen, true);
        m_backgrounds[activity][screen] = filename;
        emit backgroundChanged(activity, screen);
    }
}

void BackgroundCache::setBroadcastedBackgroundsEnabled(QString activity, QString screen, bool enabled)
{
    if (enabled && !backgroundIsBroadcasted(activity, screen)) {
        if (!m_broadcasted.contains(activity)) {
            m_broadcasted[activity] = QList<QString>();
        }

        m_broadcasted[activity].append(screen);
    } else if (!enabled && backgroundIsBroadcasted(activity, screen)) {
        m_broadcasted[activity].removeAll(screen);

        if (m_broadcasted[activity].isEmpty()) {
            m_broadcasted.remove(activity);
        }

        reload();
    }
}

bool BackgroundCache::backgroundIsBroadcasted(QString activity, QString screenName)
{
    return m_broadcasted.contains(activity) && m_broadcasted[activity].contains(screenName);
}

bool BackgroundCache::pluginExistsFor(QString activity, QString screenName)
{
    return m_plugins.contains(activity) && m_plugins[activity].contains(screenName);
}

}
}
