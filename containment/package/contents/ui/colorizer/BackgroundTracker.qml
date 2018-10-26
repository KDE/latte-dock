/*
*  Copyright 2018 Michail Vourlakos <mvourlakos@gmail.com>
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
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore


Item{
    id: tracker

    property real currentBackgroundLuminas: -1000;

    Connections{
        target: plasmoid
        onLocationChanged:{
            tracker.currentBackgroundLuminas = universalSettings.luminasFromFile(activitiesList.currentLayoutBackground, plasmoid.location);
        }
    }

    Repeater {
        id: activitiesList
        model: universalSettings ? universalSettings.runningActivitiesModel : null

        property string currentLayoutBackground: ""

        onCurrentLayoutBackgroundChanged: {
            tracker.currentBackgroundLuminas = universalSettings.luminasFromFile(currentLayoutBackground, plasmoid.location);
        }

        Item {
            id: activityItem
            visible: false

            property string activityId: model.id
            property string title: model.name
            property string background: model.background
            property bool current: model.isCurrent

            Component.onCompleted: {
                if (dockManagedLayout && forceColorizer && dockManagedLayout.lastUsedActivity === activityId) {
                    activitiesList.currentLayoutBackground = background;
                }
            }

            onBackgroundChanged: {
                if (dockManagedLayout && forceColorizer && dockManagedLayout.lastUsedActivity === activityId) {
                    activitiesList.currentLayoutBackground = background;
                }
            }

            Connections{
                target: dockManagedLayout

                onLastUsedActivityChanged:{
                    if (dockManagedLayout && forceColorizer && dockManagedLayout.lastUsedActivity === activityItem.activityId) {
                        activitiesList.currentLayoutBackground = activityItem.background;
                    }
                }
            }
        }
    }


}
