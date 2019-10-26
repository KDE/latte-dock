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

#ifndef SCHEMESTRACKER_H
#define SCHEMESTRACKER_H

// local
#include "../windowinfowrap.h"

// Qt
#include <QObject>


namespace Latte {
namespace WindowSystem {
class AbstractWindowInterface;
class SchemeColors;
}
}

namespace Latte {
namespace WindowSystem {
namespace Tracker {

class Schemes : public QObject {
    Q_OBJECT

public:
    Schemes(AbstractWindowInterface *parent);
    ~Schemes() override;

    SchemeColors *schemeForWindow(WindowId wId);
    void setColorSchemeForWindow(WindowId wId, QString scheme);

signals:
    void colorSchemeChanged(const WindowId &wid);

private slots:
    void updateDefaultScheme();

private:
    void init();

private:
     AbstractWindowInterface *m_wm;

     //! scheme file and its loaded colors
     QMap<QString, Latte::WindowSystem::SchemeColors *> m_schemes;

     //! window id and its corresponding scheme file
     QMap<WindowId, QString> m_windowScheme;
};

}
}
}

#endif
