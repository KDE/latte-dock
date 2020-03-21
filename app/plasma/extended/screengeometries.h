/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef PLASMASCREENGEOMETRIES_H
#define PLASMASCREENGEOMETRIES_H

// Qt
#include <QObject>
#include <QTimer>

#include "../../../liblatte2/types.h"

namespace Latte {
class Corona;
class View;
}

namespace Latte {
namespace PlasmaExtended {

class ScreenGeometries: public QObject
{
    Q_OBJECT

public:
    ScreenGeometries(Latte::Corona *parent);
    ~ScreenGeometries() override;

private slots:
    void availableScreenGeometryChangedFrom(Latte::View *origin);

    void init();
    void updateGeometries();

private slots:
    bool screenIsActive(const QString &screenName) const;

private:
    bool m_plasmaInterfaceAvailable{false};
    bool m_forceGeometryBroadcast{false};

    //! this is needed in order to avoid too many costly calculations for available screen geometries
    QTimer m_publishTimer;

    //! this is needed in order to check if Plasma>=5.18 is running
    QTimer m_startupInitTimer;

    Latte::Corona *m_corona{nullptr};

    QList<Latte::Types::Visibility> m_ignoreModes{
        Latte::Types::AutoHide,
        Latte::Types::SideBar
    };

    QStringList m_lastScreenNames;

    QHash<QString, QRect> m_lastAvailableRect;
    QHash<QString, QRegion> m_lastAvailableRegion;
};

}
}

#endif
