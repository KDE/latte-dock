/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.8

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0

import "./privates" as Ability

Ability.PositionShortcutsPrivate {
    id: _shortcuts
    objectName: "PositionShortcutsAbilityHost"

    //! do not update during dragging/moving applets inConfigureAppletsMode
    updateIsBlocked: (root.dragOverlay && root.dragOverlay.pressed)
                     || layouter.appletsInParentChange


    //! this is called from globalshortcuts c++ side
    function setShowAppletShortcutBadges(showPositionShortcuts, showShortcuts, showMeta, applicationLauncher){
        showPositionShortcutBadges = showPositionShortcuts;
        showAppletShortcutBadges = showShortcuts;
        showMetaBadge = showMeta;
        applicationLauncherId = applicationLauncher;
    }

    //! this is called from Latte::View::ContainmentInterface
    function activateEntryAtIndex(entryIndex) {
        if (typeof entryIndex !== "number") {
            return;
        }

        sglActivateEntryAtIndex(entryIndex);
    }

    //! this is called from Latte::View::ContainmentInterface
    function newInstanceForEntryAtIndex(entryIndex) {
        if (typeof entryIndex !== "number") {
            return;
        }

        sglNewInstanceForEntryAtIndex(entryIndex);
    }

    //! this is called from Latte::View::ContainmentInterface
    function appletIdForIndex(entryIndex) {
        return indexer.appletIdForVisibleIndex(entryIndex);
    }
}
