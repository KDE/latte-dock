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

#ifndef WINDOWSYSTEMTRACKEDLAYOUTINFO_H
#define WINDOWSYSTEMTRACKEDLAYOUTINFO_H

// local
#include "trackedgeneralinfo.h"
#include "../windowinfowrap.h"

// Qt
#include <QObject>
#include <QRect>

namespace Latte {
namespace Layout {
class GenericLayout;
}
namespace WindowSystem {
namespace Tracker {
class Windows;
}
}
}


namespace Latte {
namespace WindowSystem {
namespace Tracker {

class TrackedLayoutInfo : public TrackedGeneralInfo {
    Q_OBJECT

public:
    TrackedLayoutInfo(Tracker::Windows *tracker, Latte::Layout::GenericLayout *layout);
    ~TrackedLayoutInfo() override;

    Latte::Layout::GenericLayout *layout() const;

private:
    Latte::Layout::GenericLayout *m_layout{nullptr};
};

}
}
}

#endif
