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
#include <QImage>
#include <QRgb>

// Plasma
#include <Plasma>

namespace Latte{
namespace PlasmaExtended {

BackgroundCache::BackgroundCache(QObject *parent)
    : QObject(parent)
{
    if (!m_pool) {
        m_pool = new ScreenPool(this);
    }
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

float BackgroundCache::luminasFromFile(QString imageFile, int edge)
{
    QImage image(imageFile);

    Plasma::Types::Location location = static_cast<Plasma::Types::Location>(edge);

    if (m_luminasCache.keys().contains(imageFile)) {
        if (m_luminasCache[imageFile].keys().contains(location)) {
            return m_luminasCache[imageFile].value(location);
        }
    }

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
