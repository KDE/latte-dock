/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    readonly property int currentModeIndex: {
        for (var i=0; i<modes.length; ++i) {
            if (modes[i].pluginId === mode) {
                return i;
            }
        }

        return 0;
    }

    readonly property int firstVisibilityMode:  modes[0].pluginId
    readonly property int lastVisibilityMode: modes[modes.length - 1].pluginId

    property variant modes: []

    signal viewRelevantVisibilityModeChanged();

    Component.onCompleted: {
        reloadModel();
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
            for (var i=0; i<custom.modes.length; ++i) {
                if (custom.modes[i].pluginId === latteView.visibility.mode) {
                    custom.viewRelevantVisibilityModeChanged();
                    return;
                }
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
