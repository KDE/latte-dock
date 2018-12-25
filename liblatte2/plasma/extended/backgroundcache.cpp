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

float BackgroundCache::luminasFor(QString activity, QString screen, Plasma::Types::Location location)
{
    QString assignedBackground = background(activity, screen);

    if (!assignedBackground.isEmpty()) {
        return luminasFromFile(assignedBackground, location);
    }

    return -1000;
}

float BackgroundCache::luminasFromFile(QString imageFile, Plasma::Types::Location location)
{
    if (m_luminasCache.keys().contains(imageFile)) {
        if (m_luminasCache[imageFile].keys().contains(location)) {
            return m_luminasCache[imageFile].value(location);
        }
    }

    //! if it is a color
    if (imageFile.startsWith("#")) {
        return Latte::colorLumina(QColor(imageFile));
    }

    //! if it is a local image
    QImage image(imageFile);

    if (image.format() != QImage::Format_Invalid) {
        int maskHeight = (0.08 * image.height());
        int maskWidth = (0.05 * image.width());

        float areaLumin = -1000;

        int firstRow = 0;
        int firstColumn = 0;
        int endRow = 0;
        int endColumn = 0;

        if (location == Plasma::Types::TopEdge) {
            firstRow = 0;
            endRow = maskHeight;
            firstColumn = 0;
            endColumn = image.width() - 1;
        } else if (location == Plasma::Types::BottomEdge) {
            firstRow = image.height() - maskHeight - 1;
            endRow = image.height() - 1;
            firstColumn = 0;
            endColumn = image.width() - 1;
        } else if (location == Plasma::Types::LeftEdge) {
            firstRow = 0;
            endRow = image.height() - 1;
            firstColumn = 0;
            endColumn = maskWidth;
        } else if (location == Plasma::Types::RightEdge) {
            firstRow = 0;
            endRow = image.height() - 1;
            firstColumn = image.width() - 1 - maskWidth;
            endColumn = image.width() - 1;
        }

        for (int row = firstRow; row < endRow; ++row) {
            QRgb *line = (QRgb *)image.scanLine(row);

            for (int col = firstColumn; col < endColumn ; ++col) {
                QRgb pixelData = line[col];
                float pixelLuminosity = Latte::colorLumina(pixelData);

                areaLumin = (areaLumin == -1000) ? pixelLuminosity : (areaLumin + pixelLuminosity);
            }
        }

        float areaSize = (endRow - firstRow) * (endColumn - firstColumn);
        areaLumin = areaLumin / areaSize;

        if (!m_luminasCache.keys().contains(imageFile)) {
            m_luminasCache[imageFile] = EdgesHash();
        }

        m_luminasCache[imageFile].insert(location, areaLumin);

        return areaLumin;
    }

    //! didn't find anything
    return -1000;
}

}
}
