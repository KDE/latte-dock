import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.1 as Latte

PlasmaComponents.Page{
    width: dialog.width - 2*dialog.windowSpace

    Column{
        spacing: 1.5*theme.defaultFont.pointSize
        width: parent.width
    }
}
