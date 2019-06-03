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

#ifndef VIEWINDICATORRESOURCES_H
#define VIEWINDICATORRESOURCES_H

// Qt
#include <QObject>

namespace Latte {
namespace ViewPart {
class Indicator;
}
}

namespace Latte {
namespace ViewPart {
namespace IndicatorPart {

/**
 * Resources requested from indicator in order to reduce consumption
 **/

class Resources: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> svgs READ svgs NOTIFY svgsChanged)

public:
    Resources(Indicator *parent);
    virtual ~Resources();

    QList<QObject *> svgs() const;

public slots:
    Q_INVOKABLE void setSvgImagePaths(QStringList paths);

signals:
    void svgsChanged();

private:
    QStringList m_svgImagePaths;

    Indicator *m_indicator{nullptr};

    QList<QObject *> m_svgs;
};

}
}
}

#endif
