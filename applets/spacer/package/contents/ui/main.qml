/*
 * Copyright 2017 Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

Item{
    id: root

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    z: 9999
    property bool horizontal: plasmoid.formFactor != PlasmaCore.Types.Vertical

    property int pixelStep: 10
    property int percentageStep: 20

    property Item latteDock

    property int length: { //this is calculated in pixels
        if (latteDock && plasmoid.configuration.usePercentage) {
            return latteDock.iconSize * (plasmoid.configuration.lengthPercentage / 100);
        } else {
            return plasmoid.configuration.lengthPixels;
        }
    }

    onLengthChanged: updateValues();
    Component.onCompleted: updateValues()

    function updateValues() {
        if (horizontal) {
            Layout.minimumWidth = length;
            Layout.preferredWidth = length;
            Layout.maximumWidth = length;
        } else {
            Layout.minimumHeight = length;
            Layout.preferredHeight = length;
            Layout.maximumHeight = length;
        }
    }

    function increaseLength() {
        if (plasmoid.configuration.usePercentage)
            plasmoid.configuration.lengthPercentage = plasmoid.configuration.lengthPercentage + percentageStep;
        else
            plasmoid.configuration.lengthPixels = plasmoid.configuration.lengthPixels + pixelStep;
    }

    function decreaseLength() {
        if (plasmoid.configuration.usePercentage)
            plasmoid.configuration.lengthPercentage = Math.max(10, plasmoid.configuration.lengthPercentage - percentageStep);
        else
            plasmoid.configuration.lengthPixels = Math.max(5, plasmoid.configuration.lengthPixels - pixelStep);
    }

    Loader{
        active: latteDock && latteDock.editMode
        anchors.fill: parent

        sourceComponent: Rectangle{
            anchors.fill: parent
            border.width: 1
            border.color: theme.highlightColor
            color: alphaBackColor

            property color alphaBackColor: Qt.rgba(theme.highlightColor.r, theme.highlightColor.g, theme.highlightColor.b, 0.5)
        }
    }
}
