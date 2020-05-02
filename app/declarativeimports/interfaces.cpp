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

#include "interfaces.h"

#include <PlasmaQuick/AppletQuickItem>

namespace Latte{

Interfaces::Interfaces(QObject *parent)
    : QObject(parent)
{
}

QObject *Interfaces::globalShortcuts() const
{
    return m_globalShortcuts;
}

QObject *Interfaces::layoutsManager() const
{
    return m_layoutsManager;
}

QObject *Interfaces::themeExtended() const
{
    return m_themeExtended;
}

QObject *Interfaces::universalSettings() const
{
    return m_universalSettings;
}

QObject *Interfaces::view() const
{
    return m_view;
}

QObject *Interfaces::plasmoidInterface() const
{
    return m_plasmoid;
}

void Interfaces::setPlasmoidInterface(QObject *interface)
{
    PlasmaQuick::AppletQuickItem *plasmoid = qobject_cast<PlasmaQuick::AppletQuickItem *>(interface);

    if (plasmoid && m_plasmoid != plasmoid) {
        m_plasmoid = plasmoid;

        m_globalShortcuts = plasmoid->property("_latte_globalShortcuts_object").value<QObject *>();
        m_layoutsManager = plasmoid->property("_latte_layoutsManager_object").value<QObject *>();
        m_themeExtended = plasmoid->property("_latte_themeExtended_object").value<QObject *>();
        m_universalSettings = plasmoid->property("_latte_universalSettings_object").value<QObject *>();
        m_view = plasmoid->property("_latte_view_object").value<QObject *>();

        emit interfacesChanged();
    }
}

}
