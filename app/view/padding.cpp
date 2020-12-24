/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "padding.h"

namespace Latte {
namespace ViewPart {

Padding::Padding(QObject *parent)
    : QObject(parent)
{
}

Padding::~Padding()
{
}

bool Padding::isEmpty() const
{
    return !((m_bottom > 0) || (m_top > 0) || (m_left > 0) || (m_right > 0));
}

int Padding::top() const
{
    return m_top;
}

void Padding::setTop(int toppad)
{
    if (m_top == toppad) {
        return;
    }

    m_top = toppad;
    emit paddingsChanged();
}

int Padding::bottom() const
{
    return m_bottom;
}

void Padding::setBottom(int bottompad)
{
    if (m_bottom == bottompad) {
        return;
    }

    m_bottom = bottompad;
    emit paddingsChanged();
}

int Padding::left() const
{
    return m_left;
}

void Padding::setLeft(int leftpad)
{
    if (m_left == leftpad) {
        return;
    }

    m_left = leftpad;
    emit paddingsChanged();
}

int Padding::right() const
{
    return m_right;
}

void Padding::setRight(int rightpad)
{
    if (m_right == rightpad) {
        return;
    }

    m_right = rightpad;
    emit paddingsChanged();
}

}
}
