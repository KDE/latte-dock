/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import QtQuick.Controls.Styles.Plasma 2.0 as Styles

import org.kde.kquickcontrolsaddons 2.0 as KQuickControlAddons

import org.kde.latte.core 0.2 as LatteCore

import "../controls" as LatteExtraControls

Loader {
    active: plasmoid && plasmoid.configuration && latteView

    sourceComponent: FocusScope {
        id: dialog

        width: typeSettings.width + units.smallSpacing * 4
        height: typeSettings.height + units.smallSpacing * 4
        Layout.minimumWidth: width
        Layout.minimumHeight: height
        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        property bool panelIsVertical: plasmoid.formFactor === PlasmaCore.Types.Vertical

        PlasmaCore.FrameSvgItem{
            id: backgroundFrameSvgItem
            anchors.fill: parent
            imagePath: "dialogs/background"
            enabledBorders: viewConfig.enabledBorders

            onEnabledBordersChanged: viewConfig.updateEffects()
            Component.onCompleted: viewConfig.updateEffects()
        }

        LatteExtraControls.TypeSelection{
            id: typeSettings
            anchors.centerIn: parent

            Component.onCompleted: forceActiveFocus();

            Keys.onPressed: {
                if (event.key === Qt.Key_Escape) {
                    primaryConfigView.hideConfigWindow();
                }
            }
        }
    }
}
