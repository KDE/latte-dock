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
    buttonText: "Unity"
    comboBoxTextRole: "name"
    comboBoxIconRole: "icon"
    comboBoxBlankSpaceForEmptyIcons: true

    property string type: "org.kde.latte.unity"

    Component.onCompleted: reloadModel();

    ListModel {
        id: actionsModel
    }

    function reloadModel() {
        var actions = [];

        if (latteView.indicator.customPluginsCount > 0) {
            var pluginIds = latteView.indicator.customPluginIds;
            var pluginNames = latteView.indicator.customPluginNames;

            for(var i=0; i<pluginIds.length; ++i) {
                var element = {pluginId: pluginIds[i], name: pluginNames[i], icon: ''};
                actionsModel.append(element);
            }
        }

        //! download
        var downloadElement = {pluginId: 'more:', name: 'More...', icon: 'favorites'};
        actionsModel.append(downloadElement);

        comboBox.model = actionsModel;

        if (latteView.indicator.customPluginsCount > 0) {
            comboBox.currentIndex = 0;
        } else {
            comboBox.currentIndex = -1;
        }
    }

    function emptyModel() {
        var actions = []
        //! download
        var downloadElement = {pluginId: 'more:', name: 'More...', icon: ''};

        actionsModel.append(emptyElement);
        actionsModel.append(downloadElement);

        comboBox.model = actionsModel;
        comboBox.currentIndex = -1;
    }

}
