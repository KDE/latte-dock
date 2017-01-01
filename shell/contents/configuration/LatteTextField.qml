import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4

import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaComponents.TextField{
    id: textField
    validator: IntValidator {bottom: minValue; top: maxValue;}
    text: value === 0 ? "" : value
    font.italic: true
    placeholderText: i18n("<none>")

    property int step: 100
    property int value: 0

    property int minValue: 0
    property int maxValue: 3000


    function confirmValue(val){
        var fixedVal = Math.min(maxValue, val);

        if (fixedVal < minValue) {
            return minValue;
        } else {
            return fixedVal;
        }
    }

    onTextChanged: text !== "" ? value = parseInt(text) : value = 0;


    Row{
        // width: 4 * theme.defaultFont.pixelSize
        anchors.right: parent.right
        anchors.rightMargin: 2
        anchors.verticalCenter: parent.verticalCenter
        PlasmaComponents.Label{
            text: i18n("ms.")
            font.italic: true
            opacity: (value === 0) ? 0 : 0.6
        }
        PlasmaComponents.Label{
            text: "  "
            font.italic: true
        }
        PlasmaComponents.Button{
            width: 2*theme.defaultFont.pixelSize - 4
            height: width
            text:"-"
            onClicked: value = confirmValue(value - step);
        }
        PlasmaComponents.Button{
            width: 2*theme.defaultFont.pixelSize - 4
            height: width
            text:"+"
            onClicked: value = confirmValue(value + step);
        }
    }

    MouseArea{
        anchors.fill: parent
        acceptedButtons: Qt.MiddleButton

        onWheel: {
            var angle = wheel.angleDelta.y / 8

            if (angle>0) {
                value = confirmValue(value + step);
            } else if (angle<0){
                value = confirmValue(value - step);
            }
        }
    }

}
