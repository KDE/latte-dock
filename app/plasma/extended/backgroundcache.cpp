/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backgroundcache.h"

// local
#include "../../tools/commontools.h"

// Qt
#include <QDebug>
#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QRgb>
#include <QtMath>
#include <QLatin1String>

// Plasma
#include <Plasma>

// KDE
#include <KConfigGroup>
#include <KDirWatch>

#define MAXHASHSIZE 300

#define PLASMACONFIG "plasma-org.kde.plasma.desktop-appletsrc"
#define DEFAULTWALLPAPER "wallpapers/Next/contents/images/1920x1080.png"

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

    m_defaultWallpaperPath = Latte::standardPath(DEFAULTWALLPAPER);

    qDebug() << "Default Wallpaper path ::: " << m_defaultWallpaperPath;

    KDirWatch::self()->addFile(configFile);

    connect(KDirWatch::self(), &KDirWatch::dirty, this, &BackgroundCache::settingsFileChanged);
    connect(KDirWatch::self(), &KDirWatch::created, this, &BackgroundCache::settingsFileChanged);

    if (!m_pool) {
        m_pool = new ScreenPool(this);
        connect(m_pool, &ScreenPool::idsChanged, this, &BackgroundCache::reload);
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

QString BackgroundCache::backgroundFromConfig(const KConfigGroup &config, QString wallpaperPlugin) const
{
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

    if (type == QLatin1String("org.kde.desktopcontainment") || type == QLatin1String("org.kde.plasma.folder") ) {
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

    for (const auto &activity : updates.keys()) {
        for (const auto &screen : updates[activity]) {
            emit backgroundChanged(activity, screen);
        }
    }
}

QString BackgroundCache::background(QString activity, QString screen) const
{
    if (m_backgrounds.contains(activity) && m_backgrounds[activity].contains(screen)) {
        return m_backgrounds[activity][screen];
    } else {
        return m_defaultWallpaperPath;
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

bool BackgroundCache::areaIsBusy(float bright1, float bright2) const
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
//! The area is split in ten different Tiles and for each one its brightness
//! is computed. The brightness average from these tiles provides the entire
//! area brightness. In order to indicate if this area is busy or not we
//! compare the minimum and the maximum values of brightness from these
//! tiles. If the difference it too big then the area is busy
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

        bool vertical = (location == Plasma::Types::LeftEdge || location == Plasma::Types::RightEdge) ? true : false;
        int imageLength = !vertical ? image.width() : image.height();
        int tiles{qMin(10,imageLength)};

        //! 24px. should be enough because the views are always snapped to edges
        int tileThickness = !vertical ? qMin(24,image.height()) : qMin(24,image.width());
        int tileLength = imageLength / tiles ;

        int tileWidth = !vertical ? tileLength : tileThickness;
        int tileHeight = !vertical ? tileThickness : tileLength;

        float factor = ((float)100/tiles)/100;

        QList<float> subBrightness;

        qDebug() << "------------   -- Image Calculations --  --------------" ;
        qDebug() << "Hints for Background image | " << imageFile;
        qDebug() << "Hints for Background image | Edge: " << location << ", Image size: " << image.width() << "x" << image.height() << ", Tiles: " << tiles << ", subsize: " << tileWidth << "x" << tileHeight;

        //! Iterating algorigthm
        int firstRow = 0; int firstColumn = 0; int endRow = 0; int endColumn = 0;

        //! horizontal tiles calculations
        if (location == Plasma::Types::TopEdge) {
            firstRow = 0; endRow = tileThickness;
        } else if (location == Plasma::Types::BottomEdge) {
            firstRow = image.height() - tileThickness - 1; endRow = image.height() - 1;
        }

        if (!vertical) {
            for (int i=1; i<=tiles; ++i) {
                float subFactor = ((float)i) * factor;
                firstColumn = endColumn+1; endColumn = (subFactor*imageLength) - 1;
                endColumn = qMin(endColumn, imageLength-1);

                int tempBrightness = brightnessFromArea(image, firstRow, firstColumn, endRow, endColumn);
                qDebug() << " Tile considering horizontal << (" << firstColumn << "," << firstRow << ") - (" << endColumn << "," << endRow << "), subfactor: " << subFactor
                         << ", brightness: " << tempBrightness;

                subBrightness.append(tempBrightness);

                if (tempBrightness > maxBrightness) {
                    maxBrightness = tempBrightness;
                }
                if (tempBrightness < minBrightness) {
                    minBrightness = tempBrightness;
                }
            }
        }

        //! vertical tiles calculations
        if (location == Plasma::Types::LeftEdge) {
            firstColumn = 0; endColumn = tileThickness;
        } else if (location == Plasma::Types::RightEdge) {
            firstColumn = image.width() - 1 - tileThickness; endColumn = image.width() - 1;
        }

        if (vertical) {
            for (int i=1; i<=tiles; ++i) {
                float subFactor = ((float)i) * factor;
                firstRow = endRow+1; endRow = (subFactor*imageLength) - 1;
                endRow = qMin(endRow, imageLength-1);

                int tempBrightness = brightnessFromArea(image, firstRow, firstColumn, endRow, endColumn);
                qDebug() << " Tile considering vertical << (" << firstColumn << "," << firstRow << ") - (" << endColumn << "," << endRow << "), subfactor: " << subFactor
                         << ", brightness: " << tempBrightness;

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

        qDebug() << "Hints for Background image | Brightness: " << brightness << ", Busy: " << areaBusy << ", minBright:" << minBrightness << ", maxBright:" << maxBrightness;

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

bool BackgroundCache::backgroundIsBroadcasted(QString activity, QString screenName) const
{
    return m_broadcasted.contains(activity) && m_broadcasted[activity].contains(screenName);
}

bool BackgroundCache::pluginExistsFor(QString activity, QString screenName) const
{
    return m_plugins.contains(activity) && m_plugins[activity].contains(screenName);
}

}
}
