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

#include "activitydata.h"

namespace Latte {
namespace Settings {
namespace Data {

Activity::Activity()
{
}

Activity::Activity(Activity &&o)
    : id(o.id),
      name(o.name),
      icon(o.icon),
      isCurrent(o.isCurrent),
      state(o.state)
{
}

Activity::Activity(const Activity &o)
    : id(o.id),
      name(o.name),
      icon(o.icon),
      isCurrent(o.isCurrent),
      state(o.state)
{
}

Activity &Activity::operator=(const Activity &rhs)
{
    id = rhs.id;
    name = rhs.name;
    icon = rhs.icon;
    isCurrent = rhs.isCurrent;
    state = rhs.state;

    return (*this);
}

Activity &Activity::operator=(Activity &&rhs)
{
    id = rhs.id;
    name = rhs.name;
    icon = rhs.icon;
    isCurrent = rhs.isCurrent;
    state = rhs.state;

    return (*this);
}

bool Activity::isValid() const
{
    return (state != KActivities::Info::Invalid);
}

bool Activity::isRunning() const
{
    return ((state == KActivities::Info::Running) || (state == KActivities::Info::Starting));
}

}
}
}
