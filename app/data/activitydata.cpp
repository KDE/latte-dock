/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "activitydata.h"

namespace Latte {
namespace Data {

Activity::Activity()
    : Generic()
{
}

Activity::Activity(Activity &&o)
    : Generic(o),
      icon(o.icon),
      isCurrent(o.isCurrent),
      state(o.state)
{
}

Activity::Activity(const Activity &o)
    : Generic(o),
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
