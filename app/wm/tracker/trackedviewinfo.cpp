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

#include "trackedviewinfo.h"

//local
#include "windows.h"
#include "../schemecolors.h"
#include "../../view/view.h"


namespace Latte {
namespace WindowSystem {
namespace Tracker {


TrackedViewInfo::TrackedViewInfo(Tracker::Windows *tracker, Latte::View *view)
    : TrackedGeneralInfo(tracker, view)
{
}

TrackedViewInfo::~TrackedViewInfo()
{
}

bool TrackedViewInfo::activeWindowTouching() const
{
    return m_activeWindowTouching;
}

void TrackedViewInfo::setActiveWindowTouching(bool touching)
{
    if (m_activeWindowTouching == touching) {
        return;
    }

    m_activeWindowTouching = touching;
}

bool TrackedViewInfo::existsWindowTouching() const
{
    return m_existsWindowTouching;
}

void TrackedViewInfo::setExistsWindowTouching(bool touching)
{
    if (m_existsWindowTouching == touching) {
        return;
    }

    m_existsWindowTouching = touching;
}

QRect TrackedViewInfo::availableScreenGeometry() const
{
    return m_availableScreenGeometry;
}

void TrackedViewInfo::setAvailableScreenGeometry(QRect geometry)
{
    if (m_availableScreenGeometry == geometry) {
        return;
    }

    m_availableScreenGeometry = geometry;
}

SchemeColors *TrackedViewInfo::touchingWindowScheme() const
{
    return m_touchingWindowScheme;
}

void TrackedViewInfo::setTouchingWindowScheme(SchemeColors *scheme)
{
    if (m_touchingWindowScheme == scheme) {
        return;
    }

    m_touchingWindowScheme = scheme;
}

}
}
}
