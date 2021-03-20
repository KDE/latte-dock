/*
 * Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "viewdata.h"

namespace Latte {
namespace Data {

const char *TEMPIDPREFIX = "temp:";

View::View()
    : Generic()
{
}

View::View(View &&o)
    : Generic(o),
      onPrimary(o.onPrimary),
      screen(o.screen),
      maxLength(o.maxLength),
      edge(o.edge),
      alignment(o.alignment),
      originType(o.originType),
      originFile(o.originFile),
      originView(o.originView)
{
}

View::View(const View &o)
    : Generic(o),
      onPrimary(o.onPrimary),
      screen(o.screen),      
      maxLength(o.maxLength),
      edge(o.edge),
      alignment(o.alignment),
      originType(o.originType),
      originFile(o.originFile),
      originView(o.originView)
{
}

View &View::operator=(const View &rhs)
{
    id = rhs.id;
    name = rhs.name;
    onPrimary = rhs.onPrimary;
    screen = rhs.screen;
    maxLength = rhs.maxLength;
    edge = rhs.edge;
    alignment = rhs.alignment;
    originType = rhs.originType;
    originFile = rhs.originFile;
    originView = rhs.originView;

    return (*this);
}

View &View::operator=(View &&rhs)
{
    id = rhs.id;
    name = rhs.name;
    onPrimary = rhs.onPrimary;
    screen = rhs.screen;
    maxLength = rhs.maxLength;
    edge = rhs.edge;
    alignment = rhs.alignment;
    originType = rhs.originType;
    originFile = rhs.originFile;
    originView = rhs.originView;

    return (*this);
}

bool View::operator==(const View &rhs) const
{
    return (id == rhs.id)
            && (name == rhs.name)
            && (onPrimary == rhs.onPrimary)
            && (screen == rhs.screen)
            && (maxLength == rhs.maxLength)
            && (edge == rhs.edge)
            && (alignment == rhs.alignment)
            && (originType == rhs.originType)
            && (originFile == rhs.originFile)
            && (originView == rhs.originView);
}

bool View::operator!=(const View &rhs) const
{
    return !(*this == rhs);
}

bool View::hasViewTemplateOrigin() const
{
    return originType == OriginFromViewTemplate;
}

bool View::hasLayoutOrigin() const
{
    return originType == OriginFromLayout;
}

QString View::tempId() const
{
    if (originType == IsCreated) {
        return id;
    }

    QString tid = id;
    tid.remove(0, QString(TEMPIDPREFIX).count());
    return tid;
}

void View::setOrigin(OriginType origin, QString file, QString view)
{
    originType = origin;
    originFile = file;
    originView = view;
}



}
}
