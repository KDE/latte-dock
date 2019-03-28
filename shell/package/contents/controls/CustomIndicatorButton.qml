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

    buttonToolTip: checked ? i18n("Use %0 style for your indicators").arg(buttonText) :
                             i18n("Download indicator styles from the internet")
    comboBoxTextRole: "name"
    comboBoxIconRole: "icon"
    comboBoxBlankSpaceForEmptyIcons: true
    comboBoxForcePressed: latteView.indicator.type === type
    comboBoxPopUpAlignRight: Qt.application.layoutDirection !== Qt.RightToLeft

    property string type: ""

    Component.onCompleted: {
        reloadModel();
        updateButtonInformation();
    }

    ListModel {
        id: actionsModel
    }

    Connections{
        target: latteView.indicator
        onCustomPluginsCountChanged: {
            custom.reloadModel();
            custom.updateButtonInformation()
        }
    }

    Connections{
        target: custom.comboBox

        onActivated: {
            var item = actionsModel.get(index);
            if (item.pluginId === "add:") {
                latteView.indicator.addIndicator();
                custom.updateButtonInformation();
            }
        }
    }

    function updateButtonInformation() {
        if (latteView.indicator.customPluginsCount === 0) {
            custom.buttonText = i18n("Download");
            custom.checkable = false;
        } else {
            custom.checkable = true;
            custom.buttonText = actionsModel.get(0).name;
            custom.type = actionsModel.get(0).pluginId;
        }
    }

    function reloadModel() {
        actionsModel.clear();

        if (latteView.indicator.customPluginsCount > 0) {
            var pluginIds = latteView.indicator.customPluginIds;
            var pluginNames = latteView.indicator.customPluginNames;

            for(var i=0; i<pluginIds.length; ++i) {
                var element = {pluginId: pluginIds[i], name: pluginNames[i], icon: ''};
                actionsModel.append(element);
            }
        }

        appendDefaults();

        comboBox.model = actionsModel;

        if (custom.type === latteView.indicator.type) {
            comboBox.currentIndex = 0;
        } else {
            comboBox.currentIndex = -1;
        }
    }

    function emptyModel() {
        actionsModel.clear();
        appendDefaults();

        comboBox.model = actionsModel;
        comboBox.currentIndex = -1;
    }

    function appendDefaults() {
        //! add
        var addElement = {pluginId: 'add:', name: i18n('Add Indicator...'), icon: 'list-add'};
        actionsModel.append(addElement);

        //! download
        var downloadElement = {pluginId: 'download:', name: i18n('Get New Indicators...'), icon: 'favorites'};
        actionsModel.append(downloadElement);
    }

}
