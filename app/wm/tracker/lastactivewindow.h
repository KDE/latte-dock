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

#ifndef WINDOWSYSTEMLASTACTIVEWINDOW_H
#define WINDOWSYSTEMLASTACTIVEWINDOW_H

// local
#include "../windowinfowrap.h"

// Qt
#include <QObject>
#include <QRect>

namespace Latte {
namespace WindowSystem {
namespace Tracker {
class TrackedInfo;
}
}
}


namespace Latte {
namespace WindowSystem {
namespace Tracker {

class LastActiveWindow : public QObject {
    Q_OBJECT

    Q_PROPERTY(QVariant winId READ winId NOTIFY winIdChanged)

public:
    LastActiveWindow(TrackedInfo *parent);
    ~LastActiveWindow() override;

    QRect geometry() const;
    QVariant winId() const;


    void setInformation(const WindowInfoWrap &info);

signals:
    void geometryChanged();
    void winIdChanged();

private:
    void setGeometry(QRect geometry);
    void setWinId(QVariant winId);

private:
    QRect m_geometry;
    QVariant m_winId;

};

}
}
}

#endif
