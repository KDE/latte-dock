/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "screendata.h"

#include "../tools/commontools.h"

namespace Latte {
namespace Data {

Screen::Screen()
    : Generic(),
      hasExplicitViews(false),
      isActive(false),
      geometry(QRect(0, 0, 1920, 1080))
{
}

Screen::Screen(Screen &&o)
    : Generic(o),
      hasExplicitViews(o.hasExplicitViews),
      isActive(o.isActive),
      geometry(o.geometry)
{
}

Screen::Screen(const Screen &o)
    : Generic(o),
      hasExplicitViews(o.hasExplicitViews),
      isActive(o.isActive),
      geometry(o.geometry)
{
}

Screen::Screen(const QString &screenId, const QString &serialized)
    : Screen()
{
    init(screenId, serialized);
}

Screen &Screen::operator=(const Screen &rhs)
{
    id = rhs.id;
    name = rhs.name;
    hasExplicitViews = rhs.hasExplicitViews;
    isActive = rhs.isActive;
    geometry = rhs.geometry;

    return (*this);
}

Screen &Screen::operator=(Screen &&rhs)
{
    id = rhs.id;
    name = rhs.name;
    hasExplicitViews = rhs.hasExplicitViews;
    isActive = rhs.isActive;
    geometry = rhs.geometry;

    return (*this);
}

bool Screen::operator==(const Screen &rhs) const
{
    return (id == rhs.id)
            && (name == rhs.name)
            && (hasExplicitViews == rhs.hasExplicitViews)
            //&& (isActive == rhs.isActive) /*Disabled because this is not a data but a screen state*/
            && (geometry == rhs.geometry);
}

bool Screen::operator!=(const Screen &rhs) const
{
    return !(*this == rhs);
}

void Screen::init(const QString &screenId, const QString &serialized)
{
    QStringList parts = serialized.split(SERIALIZESPLITTER);

    id = screenId;
    name = parts[0];
    isActive = false;

    if (parts.count() > 1) {
        geometry = Latte::stringToRect(parts[1]);
    }
}

QString Screen::serialize() const
{
    QStringList result;
    result << name;
    result << Latte::rectToString(geometry);

    return result.join(SERIALIZESPLITTER);
}

}
}
