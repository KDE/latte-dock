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

#include "latteplugin.h"

// local
#include "iconitem.h"
#include "quickwindowsystem.h"
#include "types.h"

// Qt
#include <QtQml>

void LattePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.latte"));
    qmlRegisterUncreatableType<Latte::Types>(uri, 0, 2, "Types", "Latte Types uncreatable");
    qmlRegisterType<Latte::IconItem>(uri, 0, 2, "IconItem");
    qmlRegisterSingletonType<Latte::QuickWindowSystem>(uri, 0, 2, "WindowSystem", &Latte::windowsystem_qobject_singletontype_provider);
}
