/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "indicator.h"
#include "indicatorresources.h"

// Qt
#include <QDebug>
#include <QFileInfo>

// Plasma
#include <Plasma/Svg>

namespace Latte {
namespace ViewPart {
namespace IndicatorPart {

Resources::Resources(Indicator *parent) :
    QObject(parent),
    m_indicator(parent)
{
}

Resources::~Resources()
{
}

QList<QObject *> Resources::svgs() const
{
    return m_svgs;
}

void Resources::setSvgImagePaths(QStringList paths)
{
    if (m_svgImagePaths == paths) {
        return;
    }

    while (!m_svgs.isEmpty()) {
        auto svg = m_svgs[0];
        m_svgs.removeFirst();
        svg->deleteLater();
    }

    for(const auto &relPath : paths) {
        if (!relPath.isEmpty()) {
            Plasma::Svg *svg = new Plasma::Svg(this);

            bool isLocalFile = relPath.contains(".") && !relPath.startsWith("file:");

            QString adjustedPath = isLocalFile ? m_indicator->uiPath() + "/" + relPath : relPath;

            if ( !isLocalFile
                 || (isLocalFile && QFileInfo(adjustedPath).exists()) ) {
                svg->setImagePath(adjustedPath);
                m_svgs << svg;
            }
        }
    }

    emit svgsChanged();
}

}
}
}
