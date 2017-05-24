import QtQuick 2.4
import QtQuick.Layouts 1.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
Item {
    id: root
    readonly property bool horizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal
    readonly property bool planar: plasmoid.formFactor === PlasmaCore.Types.Planar

    Layout.fillWidth: horizontal ? false : true
    Layout.fillHeight: horizontal ? true : false
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

    PlasmaExtras.Heading {
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
    }

    Rectangle {
        id: sep
        color: theme.textColor
        radius: 2
        opacity: 0.5
        visible: !planar
        anchors {
            leftMargin:    horizontal ? 2 : 4
            rightMargin:   horizontal ? 2 : 4
            topMargin:    !horizontal ? 2 : 4
            bottomMargin: !horizontal ? 2 : 4
            fill: root
        }
    }
}
