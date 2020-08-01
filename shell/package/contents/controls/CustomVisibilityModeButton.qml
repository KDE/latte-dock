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

import QtQuick 2.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.components 1.0 as LatteComponents

LatteComponents.ComboBoxButton{
    id: custom
    checkable: true

    buttonText: modes[currentModeIndex].name
    buttonToolTip: modes[currentModeIndex].tooltip

    comboBoxTextRole: "name"
    comboBoxMinimumPopUpWidth: width
    comboBoxBlankSpaceForEmptyIcons: false
    comboBoxForcePressed: checked // latteView.visibility.mode === mode
    comboBoxPopUpAlignRight: Qt.application.layoutDirection !== Qt.RightToLeft
    comboBoxPopupTextHorizontalAlignment: Text.AlignHCenter

    property int mode: LatteCore.Types.WindowsGoBelow
    readonly property int currentModeIndex: Math.min(Math.max(0, mode-firstVisibilityMode), lastVisibilityMode-firstVisibilityMode)

    readonly property int firstVisibilityMode:  modes[0].pluginId
    readonly property int lastVisibilityMode: firstVisibilityMode + modes.length - 1

    property variant modes: []

    signal viewRelevantVisibilityModeChanged();

    Component.onCompleted: {
        reloadModel();
    }

    ListModel {
        id: actionsModel
    }

/*
    Connections{
        target: custom.button

        onClicked: {
            latteView.visibility.mode = custom.mode;
        }
    }*/

    Connections{
        target: custom.comboBox.popup
        onVisibleChanged: {
            if (visible) {
                custom.selectChosenType();
            }
        }
    }

    Connections {
        target: latteView.visibility
        onModeChanged: {
            if (latteView.visibility.mode >= custom.firstVisibilityMode && latteView.visibility.mode<=custom.lastVisibilityMode) {
                custom.viewRelevantVisibilityModeChanged();
            }
        }
    }

    Connections{
        target: custom.comboBox

        onActivated: {
            if (index>=0) {
                var item = actionsModel.get(index);
                latteView.visibility.mode = item.pluginId;
            }
        }
    }

    function reloadModel() {
        actionsModel.clear();
        appendDefaults();
        comboBox.model = actionsModel;
        selectChosenType();
    }

    function selectChosenType() {
        var found = false;

        for (var i=0; i<actionsModel.count; ++i) {
            if (actionsModel.get(i).pluginId === custom.mode) {
                found = true;
                custom.comboBox.currentIndex = i;
                break;
            }
        }

        if (!found) {
            custom.comboBox.currentIndex = -1;
        }
    }

    function emptyModel() {
        actionsModel.clear();
        appendDefaults();

        comboBox.model = actionsModel;
        comboBox.currentIndex = -1;
    }

    function appendDefaults() {
        for (var i=0; i<modes.length; ++i) {
            actionsModel.append(modes[i]);
        }
    }
}
