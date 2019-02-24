import QtQuick 2.4
import QtQuick.Layouts 1.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
Item {
    id: root
    readonly property bool horizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal
    readonly property bool planar: plasmoid.formFactor === PlasmaCore.Types.Planar

   // Layout.fillWidth: horizontal ? false : true
   // Layout.fillHeight: horizontal ? true : false
    Layout.minimumWidth:   horizontal ? 5.0 : -1
    Layout.maximumWidth:   horizontal ? 5.0 : -1
    Layout.minimumHeight: !horizontal ? 5.0 : -1
    Layout.maximumHeight: !horizontal ? 5.0 : -1

    Layout.preferredWidth: Layout.maximumWidth
    Layout.preferredHeight: Layout.maximumHeight

    Plasmoid.preferredRepresentation: plasmoid.fullRepresentation
    Plasmoid.backgroundHints: planar ? PlasmaCore.Types.StandardBackground : PlasmaCore.Types.NoBackground

    Component.onCompleted: {
        Plasmoid.removeAction( 'configure' )
    }

    //BEGIN Latte Dock Communicator
    property QtObject latteBridge: null // current Latte v0.9 API

    onLatteBridgeChanged: {
        if (latteBridge) {
            //plasmoid.configuration.containmentType = 2; /*Latte containment with new API*/
            latteBridge.actions.setProperty(plasmoid.id, "latteSideColoringEnabled", false);
        }
    }
    //END  Latte Dock Communicator
    //BEGIN Latte based properties
    readonly property bool enforceLattePalette: latteBridge && latteBridge.applyPalette && latteBridge.palette
    readonly property bool latteInEditMode: latteBridge && latteBridge.inEditMode
    //END Latte based properties

   /* PlasmaExtras.Heading {
        id: inf
        level: 2
        text: i18n("For all your separation tasks")
        anchors.fill: root
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment:   Text.AlignVCenter
        maximumLineCount: 2
        wrapMode: Text.WordWrap
        visible: planar
            && ( width > contentWidth && height > contentHeight )
    }*/

    Rectangle {
        id: sep
        anchors.centerIn: parent

        width: horizontal ? 1 : parent.width - 4
        height: !horizontal ? 1 : parent.height - 4

        color: enforceLattePalette ? latteBridge.palette.textColor : theme.textColor
        opacity: 0.4
        visible: !planar
    }
}
