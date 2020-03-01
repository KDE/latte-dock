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

import QtQuick 2.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.components 1.0 as LatteComponents


LatteComponents.ComboBoxButton{
    id: custom
    checkable: true

    buttonToolTip: mode === Latte.Types.SideBar ? i18n("SideBar can be shown only when the user has explicitly requested it. For example through an external applet, shortcut or script") : ""

    comboBoxTextRole: "name"
    comboBoxBlankSpaceForEmptyIcons: false
    comboBoxForcePressed: latteView.visibility.mode === mode
    comboBoxPopUpAlignRight: Qt.application.layoutDirection !== Qt.RightToLeft
    comboBoxPopupTextHorizontalAlignment: Text.AlignHCenter

    readonly property int mode: plasmoid.configuration.lastWindowsVisibilityMode

    Component.onCompleted: {
        reloadModel();
        updateButtonInformation();
    }

    ListModel {
        id: actionsModel
    }

    Connections{
        target: custom.button

        onClicked: {
            latteView.visibility.mode = custom.mode;
        }
    }

    Connections{
        target: custom.comboBox

        onActivated: {
            if (index>=0) {
                var item = actionsModel.get(index);
                plasmoid.configuration.lastWindowsVisibilityMode = parseInt(item.pluginId);
                latteView.visibility.mode = parseInt(item.pluginId);
            }

            custom.updateButtonInformation();
        }
    }

    Connections{
        target: custom.comboBox.popup
        onVisibleChanged: {
            if (visible) {
                custom.selectChosenType();
            }
        }
    }

    function updateButtonInformation() {       
        var curCustomIndex = plasmoid.configuration.lastWindowsVisibilityMode - 5;

        if (curCustomIndex>=0) {
            custom.buttonText = actionsModel.get(curCustomIndex).name;
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
            if (parseInt(actionsModel.get(i).pluginId) === custom.mode) {
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
        var windowsGoBelow = {
            pluginId: '5',
            name: i18n("Windows Go Below"),
        };

        var windowsCanCover = {
            pluginId: '6',
            name: i18n("Windows Can Cover"),

        }

        var windowsAlwaysCover = {
            pluginId: '7',
            name: i18n("Windows Always Cover"),
        }

        var sideBar = {
            pluginId: '8',
            name: i18n("On Demand SideBar")
        }

        actionsModel.append(windowsGoBelow);
        actionsModel.append(windowsCanCover);
        actionsModel.append(windowsAlwaysCover);
        actionsModel.append(sideBar);
    }

}
