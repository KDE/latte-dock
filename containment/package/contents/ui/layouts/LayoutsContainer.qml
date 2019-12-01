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

import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.latte 0.2 as Latte

import "../../code/HeuristicTools.js" as HeuristicTools

Item{
    id: layoutsContainer

    readonly property bool isHidden: root.inStartup || (latteView && latteView.visibility && latteView.visibility.isHidden)
    readonly property bool useMaxLength: (plasmoid.configuration.panelPosition === Latte.Types.Justify && !root.inConfigureAppletsMode)
    /*   && ((!root.inConfigureAppletsMode && !root.behaveAsPlasmaPanel )
                                             || (behaveAsPlasmaPanel && root.inConfigureAppletsMode))*/

    property int allCount: root.latteApplet ? _mainLayout.count-1+latteApplet.tasksCount : _mainLayout.count
    property int currentSpot: -1000
    property int hoveredIndex: -1

    readonly property int fillApplets: _startLayout.fillApplets + _mainLayout.fillApplets + _endLayout.fillApplets

    property Item startLayout : _startLayout
    property Item mainLayout: _mainLayout
    property Item endLayout: _endLayout

    Binding {
        target: layoutsContainer
        property: "x"
        value: {
            if ( latteView && root.isHorizontal && useMaxLength ){
                return ((latteView.width/2) - (root.maxLength/2) + root.offset);
            } else {
                if ((visibilityManager.inSlidingIn || visibilityManager.inSlidingOut) && root.isVertical){
                    return;
                }

                if (layoutsContainer.isHidden && root.isVertical) {
                    if (Latte.WindowSystem.compositingActive) {
                        return visibilityManager.slidingOutToPos;
                    } else {
                        if ((plasmoid.location===PlasmaCore.Types.LeftEdge)||(plasmoid.location===PlasmaCore.Types.TopEdge)) {
                            return visibilityManager.slidingOutToPos + 1;
                        } else {
                            return visibilityManager.slidingOutToPos - 1;
                        }
                    }
                } else {
                    return 0;
                }
            }
        }
    }

    Binding{
        target: layoutsContainer
        property: "y"
        value: {
            if ( latteView && root.isVertical && useMaxLength ) {
                return ((latteView.height/2) - (root.maxLength/2) + root.offset);
            } else {
                if ((visibilityManager.inSlidingIn || visibilityManager.inSlidingOut) && root.isHorizontal){
                    return;
                }

                if (layoutsContainer.isHidden && root.isHorizontal) {
                    if (Latte.WindowSystem.compositingActive) {
                        return visibilityManager.slidingOutToPos;
                    } else {
                        if ((plasmoid.location===PlasmaCore.Types.LeftEdge)||(plasmoid.location===PlasmaCore.Types.TopEdge)) {
                            return visibilityManager.slidingOutToPos + 1;
                        } else {
                            return visibilityManager.slidingOutToPos - 1;
                        }
                    }
                } else {
                    return 0;
                }
            }
        }
    }

    width:  root.isHorizontal && useMaxLength ? root.maxLength : parent.width
    height: root.isVertical && useMaxLength ? root.maxLength : parent.height
    z:10

    property bool animationSent: false
    property bool shouldCheckHalfs: (plasmoid.configuration.panelPosition === Latte.Types.Justify) && (_mainLayout.children>1)

    property int contentsWidth: _startLayout.width + _mainLayout.width + _endLayout.width
    property int contentsHeight: _startLayout.height + _mainLayout.height + _endLayout.height

    onContentsWidthChanged: {
        if (root.isHorizontal){
            var firstHalfExited = false;
            var secondHalfExited = false;

            if (shouldCheckHalfs){
                firstHalfExited = ( (_startLayout.width + _mainLayout.width/2) >= root.maxLength/2 );
                secondHalfExited = ( (_endLayout.width + _mainLayout.width/2) >= root.maxLength/2 );
            }

            if (latteView && ((contentsWidth >= root.maxLength) || firstHalfExited || secondHalfExited)) {
                automaticItemSizer.updateAutomaticIconSize();
            }

            if (!animationSent) {
                animationSent = true;
                slotAnimationsNeedLength(1);
            }

            layoutsContainer.updateSizeForAppletsInFill();

            delayUpdateMaskArea.start();
        }
    }

    onContentsHeightChanged: {
        if (root.isVertical){
            var firstHalfExited = false;
            var secondHalfExited = false;

            if (shouldCheckHalfs){
                firstHalfExited = ( (_startLayout.height + _mainLayout.height/2) >= root.maxLength/2 );
                secondHalfExited = ( (_endLayout.height + _mainLayout.height/2) >= root.maxLength/2 );
            }

            if (latteView && ((contentsHeight >= root.maxLength) || firstHalfExited || secondHalfExited)) {
                automaticItemSizer.updateAutomaticIconSize();
            }

            if (!animationSent) {
                animationSent = true;
                slotAnimationsNeedLength(1);
            }

            layoutsContainer.updateSizeForAppletsInFill();

            delayUpdateMaskArea.start();
        }
    }

    onXChanged: root.updateEffectsArea();
    onYChanged: root.updateEffectsArea();

    EnvironmentActions {
        active: root.scrollAction !== Latte.Types.ScrollNone || root.dragActiveWindowEnabled
    }

    AppletsContainer {
        id: _startLayout
        beginIndex: 0
        offset: root.totalPanelEdgeSpacing/2
        alignment: {
            switch(plasmoid.location) {
            case PlasmaCore.Types.BottomEdge: return Latte.Types.BottomEdgeLeftAlign;
            case PlasmaCore.Types.TopEdge: return Latte.Types.TopEdgeLeftAlign;
            case PlasmaCore.Types.LeftEdge: return Latte.Types.LeftEdgeTopAlign;
            case PlasmaCore.Types.RightEdge: return Latte.Types.RightEdgeTopAlign;
            }

            return Latte.Types.BottomEdgeLeftAlign;
        }
    }

    AppletsContainer {
        id: _mainLayout
        beginIndex: 100
        offset: centered ? appliedOffset : root.offsetFixed

        readonly property bool centered: (root.panelAlignment === Latte.Types.Center) || (root.panelAlignment === Latte.Types.Justify)
        readonly property bool reversed: Qt.application.layoutDirection === Qt.RightToLeft
        readonly property int appliedOffset: root.panelAlignment === Latte.Types.Justify ? 0 : root.offset

        alignment: {
            if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                if (centered) return Latte.Types.LeftEdgeCenterAlign;
                if (root.panelAlignment === Latte.Types.Top) return Latte.Types.LeftEdgeTopAlign;
                if (root.panelAlignment === Latte.Types.Bottom) return Latte.Types.LeftEdgeBottomAlign;
            }

            if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                if (centered) return Latte.Types.RightEdgeCenterAlign;
                if (root.panelAlignment === Latte.Types.Top) return Latte.Types.RightEdgeTopAlign;
                if (root.panelAlignment === Latte.Types.Bottom) return Latte.Types.RightEdgeBottomAlign;
            }

            if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                if (centered) return Latte.Types.BottomEdgeCenterAlign;

                if ((root.panelAlignment === Latte.Types.Left && !reversed)
                        || (root.panelAlignment === Latte.Types.Right && reversed)) {
                    return Latte.Types.BottomEdgeLeftAlign;
                }

                if ((root.panelAlignment === Latte.Types.Right && !reversed)
                        || (root.panelAlignment === Latte.Types.Left && reversed)) {
                    return Latte.Types.BottomEdgeRightAlign;
                }
            }

            if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                if (centered) return Latte.Types.TopEdgeCenterAlign;

                if ((root.panelAlignment === Latte.Types.Left && !reversed)
                        || (root.panelAlignment === Latte.Types.Right && reversed)) {
                    return Latte.Types.TopEdgeLeftAlign;
                }

                if ((root.panelAlignment === Latte.Types.Right && !reversed)
                        || (root.panelAlignment === Latte.Types.Left && reversed)) {
                    return Latte.Types.TopEdgeRightAlign;
                }
            }

            return Latte.Types.BottomEdgeCenterAlign;
        }

        transitions: Transition {
            enabled: editModeVisual.plasmaEditMode
            AnchorAnimation {
                duration: 0.8 * root.animationTime
                easing.type: Easing.OutCubic
            }
        }
    }

    AppletsContainer {
        id: _endLayout
        beginIndex: 200
        offset: root.totalPanelEdgeSpacing/2
        alignment: {
            switch(plasmoid.location) {
            case PlasmaCore.Types.BottomEdge: return Latte.Types.BottomEdgeRightAlign;
            case PlasmaCore.Types.TopEdge: return Latte.Types.TopEdgeRightAlign;
            case PlasmaCore.Types.LeftEdge: return Latte.Types.LeftEdgeBottomAlign;
            case PlasmaCore.Types.RightEdge: return Latte.Types.RightEdgeBottomAlign;
            }

            return Latte.Types.BottomEdgeLeftAlign;
        }
    }

    function updateSizeForAppletsInFill() {
        if (!updateSizeForAppletsInFillTimer.running) {
            updateSizeForAppletsInFillTimer.start();
        }
    }

    //! This timer is needed in order to reduce the calls to heavy cpu function
    //! HeuristicTools.updateSizeForAppletsInFill()
    Timer{
        id: updateSizeForAppletsInFillTimer
        interval: 10
        onTriggered: HeuristicTools.updateSizeForAppletsInFill();
    }
}
