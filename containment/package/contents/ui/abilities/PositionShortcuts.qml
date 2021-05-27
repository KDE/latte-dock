/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
