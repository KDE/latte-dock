/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: mainItem

    width: 200
    height: 200

    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)

    property alias cfg_usePixels: usePixels.checked
    property alias cfg_lengthPixels: lengthPixels.text
    property alias cfg_usePercentage: usePercentage.checked
    property alias cfg_lengthPercentage: lengthPercentage.text

    ColumnLayout {
        id:mainColumn
        spacing: 15
        width: parent.width-40

        //Layout.fillWidth: true

        GridLayout{
            id: animationsGridLayout
            //Layout.fillWidth: true
            columns: 3

            CheckBox {
                id: usePixels
                text: i18n("Use pixels size, ")

                onCheckedChanged: {
                    if (checked && usePercentage.checked) {
                        usePercentage.checked = false;
                    }
                }
            }

            TextField {
                id: lengthPixels
                maximumLength: 4
            }

            Label{
                text: "px."
            }


            CheckBox {
                id: usePercentage
                text: i18n("Use percentage, ")

                onCheckedChanged: {
                    if (checked && usePixels.checked) {
                        usePixels.checked = false;
                    }
                }
            }

            TextField {
                id: lengthPercentage
                maximumLength: 4
            }

            Label{
                text: "%"
            }
        }
    }
}
