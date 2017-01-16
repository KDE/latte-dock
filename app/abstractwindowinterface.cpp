/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "abstractwindowinterface.h"
#include "xwindowinterface.h"

#include <QObject>
#include <QQuickWindow>

#include <KWindowSystem>

namespace Latte {

AbstractWindowInterface::AbstractWindowInterface(QQuickWindow *const view, QObject *parent)
    : QObject(parent), m_view(view)
{
}

AbstractWindowInterface::~AbstractWindowInterface()
{
}

AbstractWindowInterface *AbstractWindowInterface::getInstance(QQuickWindow *const view, QObject *parent)
{
    if (KWindowSystem::isPlatformWayland()) {
        //! TODO: WaylandWindowInterface
        return nullptr;
    }

    /* if(KWindowSystem::isPlatformX11) */
    return new XWindowInterface(view, parent);
}

}
