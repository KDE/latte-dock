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

#include "windowsystem.h"

#include <KWindowSystem>

namespace Latte {

WindowSystem::WindowSystem(QObject *parent) :
    QObject(parent)
{
    if (KWindowSystem::isPlatformWayland()) {
        //! TODO: Wayland compositing active
    } else {
        compositingChangedProxy(KWindowSystem::self()->compositingActive());
        connect(KWindowSystem::self(), &KWindowSystem::compositingChanged
                , this, &WindowSystem::compositingChangedProxy);
    }
}

WindowSystem::~WindowSystem()
{
}

WindowSystem &WindowSystem::self()
{
    static WindowSystem ws;
    return ws;
}

bool WindowSystem::compositingActive() const
{
    return m_compositing;
}

void WindowSystem::skipTaskBar(const QDialog &dialog) const
{
    if (KWindowSystem::isPlatformWayland()) {
        //! TODO: Wayland skip task bar
    } else {
        KWindowSystem::setState(dialog.winId(), NET::SkipTaskbar);
    }
}

void WindowSystem::compositingChangedProxy(bool enable)
{
    if (m_compositing == enable)
        return;

    m_compositing = enable;
    emit compositingChanged();
}

} //end of namespace
