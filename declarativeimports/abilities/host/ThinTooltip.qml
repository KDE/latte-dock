/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.abilities.definition 0.1 as AbilityDefinition

AbilityDefinition.ThinTooltip {
    id: _thinTooltip
    property Item debug: null

    readonly property int maxCharacters: 80

    property Item lastHidingVisualParent: null

    //! Public API
    readonly property Item publicApi: Item {
        readonly property alias isEnabled: _thinTooltip.isEnabled
        readonly property alias currentText: _thinTooltip.currentText
        readonly property alias currentVisualParent: _thinTooltip.currentVisualParent

        function show(visualParent, text) {
            _thinTooltip.show(visualParent, text);
        }

        function hide(visualParent) {
            _thinTooltip.hide(visualParent);
        }
    }

    //! Private Implementation

    onShowIsBlockedChanged: {       
        if (isEnabled && !showIsBlocked && !_tooltipDialog.visible && _thinTooltip.currentVisualParent) {
            _hideTimer.stop();
            _tooltipDialog.visible = true;
        } else if (showIsBlocked && _tooltipDialog.visible) {
            _tooltipDialog.visible = false;
        }
    }

    function show(visualParent, text) {
        if (!isEnabled || showIsBlocked) {
            _hideTimer.stop();
            _showTimer.stop();
            _tooltipDialog.visible = false;
            //disabling because we need to updated currentvisualparent even when tooltip is blocked
            //for example when triggering a different applet popup
            //return;
        }

        _hideTimer.stop();
        _thinTooltip.currentVisualParent = visualParent;
        _tooltipDialog.visualParent = visualParent;

        var fixedDisplayText = text.length>maxCharacters ? text.substring(0,maxCharacters-1) + "..." : text;
        _thinTooltip.currentText = fixedDisplayText;

        if (!_tooltipDialog.visible && !showIsBlocked) {
            _showTimer.start();
        }
    }

    function hide(visualParent) {
        if (_thinTooltip.currentVisualParent === visualParent) {
            _thinTooltip.lastHidingVisualParent = visualParent;
            _showTimer.stop();
            _hideTimer.start();
        }
    }

    //! Show Delayer Timer
    Timer {
        id: _showTimer
        interval: 100
        onTriggered: {
            if (_thinTooltip.currentVisualParent) {
                _tooltipDialog.visible = true;
            }

            if (debug && debug.timersEnabled) {
                console.log("ThinTooltip host timer: show() called...");
            }
        }
    }

    //! Hide Delayer Timer
    Timer {
        id: _hideTimer
        interval: 100
        onTriggered: {
            if (_thinTooltip.lastHidingVisualParent === _thinTooltip.currentVisualParent) {
                _tooltipDialog.visible = false;
                _thinTooltip.lastHidingVisualParent = null;
                _thinTooltip.currentVisualParent = null;
                _thinTooltip.currentText = "";
            }

            if (debug && debug.timersEnabled) {
                console.log("ThinTooltip host timer: hide() called...");
            }
        }
    }

    LatteCore.Dialog{
        id: _tooltipDialog
        type: PlasmaCore.Dialog.Tooltip
        flags: Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | Qt.ToolTip

        location: plasmoid.location
        edge: plasmoid.location
        mainItem: RowLayout{
            Layout.fillWidth: true
            Layout.fillHeight: true
            PlasmaComponents.Label{
                Layout.leftMargin: 4
                Layout.rightMargin: 4
                Layout.topMargin: 2
                Layout.bottomMargin: 2
                text: _thinTooltip.currentText
            }
        }

        visible: false
    }
}
