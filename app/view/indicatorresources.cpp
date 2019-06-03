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

#include "indicatorresources.h"

// Qt
#include <QDebug>

// Plasma
#include <Plasma/Svg>

namespace Latte {
namespace ViewPart {
namespace IndicatorPart {

Resources::Resources(QObject *parent) :
    QObject(parent)
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

    for(const auto &path : paths) {
        if (!path.isEmpty()) {
            Plasma::Svg *svg = new Plasma::Svg(this);
            svg->setImagePath(path);
            m_svgs << svg;
        }
    }

    emit svgsChanged();
}

}
}
}
