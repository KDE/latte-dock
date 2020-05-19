/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.7

Item {
    objectName: "PositionShortcuts"

    property bool unifiedGlobalShortcuts: true

    property bool showLatteShortcutBadges: false
    property bool showAppletShortcutBadges: false
    property bool showMetaBadge: false
    property int applicationLauncherId: -1

    signal sglActivateEntryAtIndex(int entryIndex);
    signal sglNewInstanceForEntryAtIndex(int entryIndex);

    //! this is called from globalshortcuts c++ side
    function setShowAppletShortcutBadges(showLatteShortcuts, showShortcuts, showMeta, applicationLauncher){
        if (latteApplet) {
            var base = unifiedGlobalShortcuts ? parabolicManager.pseudoAppletIndex(latteAppletPos) : 1;
            latteApplet.setTasksBaseIndex(base - 1);
            latteApplet.setShowTaskShortcutBadges(showLatteShortcuts);
        }

        showLatteShortcutBadges = showLatteShortcuts;
        showAppletShortcutBadges = showShortcuts;
        showMetaBadge = showMeta;
        applicationLauncherId = applicationLauncher;

        if (latteApplet) {
            latteApplet.parabolicManager.updateTasksEdgesIndexes();
        }
    }

    //! this is called from Latte::View::ContainmentInterface
    function activateEntryAtIndex(index) {
        if (typeof index !== "number") {
            return;
        }

        if (latteApplet) {
            var base = unifiedGlobalShortcuts ? parabolicManager.pseudoAppletIndex(latteAppletPos) : 1;
            latteApplet.setTasksBaseIndex(base - 1);
            latteApplet.parabolicManager.updateTasksEdgesIndexes();
        }

        signalActivateEntryAtIndex(index);
    }

    //! this is called from Latte::View::ContainmentInterface
    function newInstanceForEntryAtIndex(index) {
        if (typeof index !== "number") {
            return;
        }

        if (latteApplet) {
            var base = unifiedGlobalShortcuts ? parabolicManager.pseudoAppletIndex(latteAppletPos) : 1;
            latteApplet.setTasksBaseIndex(base - 1);
            latteApplet.parabolicManager.updateTasksEdgesIndexes();
        }

        signalNewInstanceForEntryAtIndex(index);
    }

    //! this is called from Latte::View::ContainmentInterface
    function appletIdForIndex(index) {
        if (!root.unifiedGlobalShortcuts || parabolicManager.pseudoIndexBelongsToLatteApplet(index)) {
            return -1;
        }

        for (var i=0; i<layoutsContainer.startLayout.children.length; ++i){
            var appletItem = layoutsContainer.startLayout.children[i];

            if (appletItem && appletItem.refersEntryIndex(index)) {
                return appletItem.applet.id;
            }
        }

        for (var j=0; j<layoutsContainer.mainLayout.children.length; ++j){
            var appletItem2 = layoutsContainer.mainLayout.children[j];

            if (appletItem2 && appletItem2.refersEntryIndex(index)) {
                return appletItem2.applet.id;
            }
        }

        for (var k=0; j<layoutsContainer.endLayout.children.length; ++k){
            var appletItem3 = layoutsContainer.endLayout.children[k];

            if (appletItem3 && appletItem3.refersEntryIndex(index)) {
                return appletItem3.applet.id;
            }
        }

        return -1;
    }
}
