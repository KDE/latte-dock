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

#include "trackedlayoutinfo.h"

//local
#include "trackerwindows.h"
#include "../../layout/genericlayout.h"

namespace Latte {
namespace WindowSystem {
namespace Tracker {


TrackedLayoutInfo::TrackedLayoutInfo(Tracker::Windows *tracker, Latte::Layout::GenericLayout *layout)
    : TrackedGeneralInfo(tracker),
      m_layout(layout)
{
    m_activities = m_layout->appliedActivities();

    connect(m_layout, &Latte::Layout::GenericLayout::activitiesChanged, this, [&]() {
        m_activities = m_layout->appliedActivities();
        updateTrackingCurrentActivity();
    });
}

TrackedLayoutInfo::~TrackedLayoutInfo()
{
}

Latte::Layout::GenericLayout *TrackedLayoutInfo::layout() const
{
    return m_layout;
}

}
}
}
