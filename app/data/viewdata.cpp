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

View::View()
    : Generic()
{
}

View::View(View &&o)
    : Generic(o),
      isActive(o.isActive),
      isMoveOrigin(o.isMoveOrigin),
      isMoveDestination(o.isMoveDestination),
      onPrimary(o.onPrimary),
      screen(o.screen),
      screenEdgeMargin(o.screenEdgeMargin),
      maxLength(o.maxLength),
      edge(o.edge),
      alignment(o.alignment),
      m_state(o.m_state),
      m_originFile(o.m_originFile),
      m_originLayout(o.m_originLayout),
      m_originView(o.m_originView),
      subcontainments(o.subcontainments)
{
}

View::View(const View &o)
    : Generic(o),
      isActive(o.isActive),
      isMoveOrigin(o.isMoveOrigin),
      isMoveDestination(o.isMoveDestination),
      onPrimary(o.onPrimary),
      screen(o.screen),
      screenEdgeMargin(o.screenEdgeMargin),
      maxLength(o.maxLength),
      edge(o.edge),
      alignment(o.alignment),
      m_state(o.m_state),
      m_originFile(o.m_originFile),
      m_originLayout(o.m_originLayout),
      m_originView(o.m_originView),
      subcontainments(o.subcontainments)
{
}

View &View::operator=(const View &rhs)
{
    id = rhs.id;
    name = rhs.name;
    isActive = rhs.isActive;
    isMoveOrigin = rhs.isMoveOrigin;
    isMoveDestination = rhs.isMoveDestination;
    onPrimary = rhs.onPrimary;
    screen = rhs.screen;
    screenEdgeMargin = rhs.screenEdgeMargin,
    maxLength = rhs.maxLength;
    edge = rhs.edge;
    alignment = rhs.alignment;
    m_state = rhs.m_state;
    m_originFile = rhs.m_originFile;
    m_originLayout == rhs.m_originLayout;
    m_originView = rhs.m_originView;
    subcontainments = rhs.subcontainments;

    return (*this);
}

View &View::operator=(View &&rhs)
{
    id = rhs.id;
    name = rhs.name;
    isActive = rhs.isActive;
    isMoveOrigin = rhs.isMoveOrigin;
    isMoveDestination = rhs.isMoveDestination;
    onPrimary = rhs.onPrimary;
    screen = rhs.screen;
    screenEdgeMargin = rhs.screenEdgeMargin,
    maxLength = rhs.maxLength;
    edge = rhs.edge;
    alignment = rhs.alignment;
    m_state = rhs.m_state;
    m_originFile = rhs.m_originFile;
    m_originLayout = rhs.m_originLayout;
    m_originView = rhs.m_originView;
    subcontainments = rhs.subcontainments;

    return (*this);
}

bool View::operator==(const View &rhs) const
{
    return (id == rhs.id)
            && (name == rhs.name)
            //&& (isActive == rhs.isActive) /*activeness is a state and is not considered*/
            //&& (isMoveOrigin == rhs.isMoveOrigin) /*changing layouts is a state and is not considered*/
            //&& (isMoveDestination == rhs.isMoveDestination) /*changing layouts is a state and is not considered*/
            && (onPrimary == rhs.onPrimary)
            && (screen == rhs.screen)
            && (screenEdgeMargin == rhs.screenEdgeMargin)
            && (maxLength == rhs.maxLength)
            && (edge == rhs.edge)
            && (alignment == rhs.alignment)
            && (m_state == rhs.m_state)
            && (m_originFile == rhs.m_originFile)
            && (m_originLayout == rhs.m_originLayout)
            && (m_originView == rhs.m_originView)
            && (subcontainments == rhs.subcontainments);
}

bool View::operator!=(const View &rhs) const
{
    return !(*this == rhs);
}

View::operator QString() const
{
    QString result;

    result += id;
    result +=" : ";
    result += isActive ? "Active" : "Inactive";
    result += " : ";

    if (m_state == IsInvalid) {
        result += "IsInvalid";
    } else if (m_state == IsCreated) {
        result += "IsCreated";
    } else if (m_state == OriginFromViewTemplate) {
        result += "OriginFromViewTemplate";
    } else if (m_state == OriginFromLayout) {
        result += "OriginFromLayout";
    }

    result += " : ";
    result += onPrimary ? "Primary" : "Explicit";
    result += " : ";
    result += QString::number(screen);
    result += " : ";
    if (edge == Plasma::Types::BottomEdge) {
        result += "BottomEdge";
    } else if (edge == Plasma::Types::TopEdge) {
        result += "TopEdge";
    } else if (edge == Plasma::Types::LeftEdge) {
        result += "LeftEdge";
    } else if (edge == Plasma::Types::RightEdge) {
        result += "RightEdge";
    }

    result += " : ";

    if (alignment == Latte::Types::Center) {
        result += "CenterAlignment";
    } else if (alignment == Latte::Types::Left) {
        result += "LeftAlignment";
    } else if (alignment == Latte::Types::Right) {
        result += "RightAlignment";
    } else if (alignment == Latte::Types::Top) {
        result += "TopAlignment";
    } else if (alignment == Latte::Types::Bottom) {
        result += "BottomAlignment";
    } else if (alignment == Latte::Types::Justify) {
        result += "JustifyAlignment";
    }

    result += " : ";
    result += QString::number(maxLength) + "%";

    result += " || ";
    result += "{" + subcontainments + "}";

    return result;
}

bool View::isCreated() const
{
    return m_state == IsCreated;
}

bool View::isValid() const
{
    return m_state != IsInvalid;
}

bool View::isHorizontal() const
{
    return !isVertical();
}

bool View::isVertical() const
{
    return (edge == Plasma::Types::LeftEdge || edge == Plasma::Types::RightEdge);
}

bool View::hasViewTemplateOrigin() const
{
    return m_state == OriginFromViewTemplate;
}

bool View::hasLayoutOrigin() const
{
    return m_state == OriginFromLayout;
}

bool View::hasSubContainment(const QString &subId) const
{
    return subcontainments.containsId(subId);
}

QString View::originFile() const
{
    return m_originFile;
}

QString View::originLayout() const
{
    return m_originLayout;
}

QString View::originView() const
{
    return m_originView;
}

View::State View::state() const
{
    return m_state;
}

void View::setState(View::State state, QString file, QString layout, QString view)
{
    m_state = state;
    m_originFile = file;
    m_originLayout = layout;
    m_originView = view;
}

}
}
