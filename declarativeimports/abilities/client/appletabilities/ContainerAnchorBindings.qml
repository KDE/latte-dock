/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

Item {
    //! ListView Layout States
    states: [
        ///Bottom Edge
        State {
            name: "bottomCenter"
            when: (_abilityContainer.location===PlasmaCore.Types.BottomEdge && _abilityContainer.alignment===LatteCore.Types.Center)

            AnchorChanges {
                target: anchorsLoader.anchoredItem
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "bottomLeft"
            when: (_abilityContainer.location===PlasmaCore.Types.BottomEdge && _abilityContainer.alignment===LatteCore.Types.Left)

            AnchorChanges {
                target: anchorsLoader.anchoredItem
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "bottomRight"
            when: (_abilityContainer.location===PlasmaCore.Types.BottomEdge && _abilityContainer.alignment===LatteCore.Types.Right)

            AnchorChanges {
                target: anchorsLoader.anchoredItem
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        ///Top Edge
        State {
            name: "topCenter"
            when: (_abilityContainer.location===PlasmaCore.Types.TopEdge && _abilityContainer.alignment===LatteCore.Types.Center)

            AnchorChanges {
                target: anchorsLoader.anchoredItem
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:undefined; horizontalCenter:parent.horizontalCenter; verticalCenter:undefined}
            }
        },
        State {
            name: "topLeft"
            when: (_abilityContainer.location===PlasmaCore.Types.TopEdge && _abilityContainer.alignment===LatteCore.Types.Left)

            AnchorChanges {
                target: anchorsLoader.anchoredItem
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "topRight"
            when: (_abilityContainer.location===PlasmaCore.Types.TopEdge && _abilityContainer.alignment===LatteCore.Types.Right)

            AnchorChanges {
                target: anchorsLoader.anchoredItem
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        ////Left Edge
        State {
            name: "leftCenter"
            when: (_abilityContainer.location===PlasmaCore.Types.LeftEdge && _abilityContainer.alignment===LatteCore.Types.Center)

            AnchorChanges {
                target: anchorsLoader.anchoredItem
                anchors{ top:undefined; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
        },
        State {
            name: "leftTop"
            when: (_abilityContainer.location===PlasmaCore.Types.LeftEdge && _abilityContainer.alignment===LatteCore.Types.Top)

            AnchorChanges {
                target: anchorsLoader.anchoredItem
                anchors{ top:parent.top; bottom:undefined; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "leftBottom"
            when: (_abilityContainer.location===PlasmaCore.Types.LeftEdge && _abilityContainer.alignment===LatteCore.Types.Bottom)

            AnchorChanges {
                target: anchorsLoader.anchoredItem
                anchors{ top:undefined; bottom:parent.bottom; left:parent.left; right:undefined; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        ///Right Edge
        State {
            name: "rightCenter"
            when: (_abilityContainer.location===PlasmaCore.Types.RightEdge && _abilityContainer.alignment===LatteCore.Types.Center)

            AnchorChanges {
                target: anchorsLoader.anchoredItem
                anchors{ top:undefined; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:parent.verticalCenter}
            }
        },
        State {
            name: "rightTop"
            when: (_abilityContainer.location===PlasmaCore.Types.RightEdge && _abilityContainer.alignment===LatteCore.Types.Top)

            AnchorChanges {
                target: anchorsLoader.anchoredItem
                anchors{ top:parent.top; bottom:undefined; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
        },
        State {
            name: "rightBottom"
            when: (_abilityContainer.location===PlasmaCore.Types.RightEdge && _abilityContainer.alignment===LatteCore.Types.Bottom)

            AnchorChanges {
                target: anchorsLoader.anchoredItem
                anchors{ top:undefined; bottom:parent.bottom; left:undefined; right:parent.right; horizontalCenter:undefined; verticalCenter:undefined}
            }
        }
    ]
}
