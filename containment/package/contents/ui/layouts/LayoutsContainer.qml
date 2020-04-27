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

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.private.containment 0.1 as LatteContainment

import "../../code/HeuristicTools.js" as HeuristicTools

Item{
    id: layoutsContainer

    readonly property bool isHidden: root.inStartup || (latteView && latteView.visibility && latteView.visibility.isHidden)
    readonly property bool useMaxLength: (plasmoid.configuration.alignment === LatteCore.Types.Justify && !root.inConfigureAppletsMode)
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
            if (visibilityManager.inLocationAnimation) {
                return;
            }

            if (root.behaveAsPlasmaPanel) {
                return 0;
            }

            if ( latteView && root.isHorizontal && useMaxLength ){
                return ((latteView.width/2) - (root.maxLength/2) + root.offset);
            } else {
                if ((visibilityManager.inSlidingIn || visibilityManager.inSlidingOut) && root.isVertical){
                    return;
                }

                if (layoutsContainer.isHidden && root.isVertical) {
                    if (LatteCore.WindowSystem.compositingActive) {
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
            if (visibilityManager.inLocationAnimation) {
                return;
            }

            if (root.behaveAsPlasmaPanel) {
                return 0;
            }

            if ( latteView && root.isVertical && useMaxLength ) {
                return ((latteView.height/2) - (root.maxLength/2) + root.offset);
            } else {
                if ((visibilityManager.inSlidingIn || visibilityManager.inSlidingOut) && root.isHorizontal){
                    return;
                }

                if (layoutsContainer.isHidden && root.isHorizontal) {
                    if (LatteCore.WindowSystem.compositingActive) {
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
    property bool shouldCheckHalfs: (plasmoid.configuration.alignment === LatteCore.Types.Justify) && (_mainLayout.children>1)

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
                autosize.updateIconSize();
            }

            if (!animationSent) {
                animationSent = true;
                animations.needLength.addEvent(layoutsContainer);
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
                autosize.updateIconSize();
            }

            if (!animationSent) {
                animationSent = true;
                animations.needLength.removeEvent(layoutsContainer);
            }

            layoutsContainer.updateSizeForAppletsInFill();

            delayUpdateMaskArea.start();
        }
    }

    onXChanged: root.updateEffectsArea();
    onYChanged: root.updateEffectsArea();

    EnvironmentActions {
        active: root.scrollAction !== LatteContainment.Types.ScrollNone || root.dragActiveWindowEnabled || root.closeActiveWindowEnabled
        alignment: _mainLayout.alignment
    }

    AppletsContainer {
        id: _startLayout
        beginIndex: 0
        offset: root.totalPanelEdgeSpacing/2
        alignment: {
            switch(plasmoid.location) {
            case PlasmaCore.Types.BottomEdge: return LatteCore.Types.BottomEdgeLeftAlign;
            case PlasmaCore.Types.TopEdge: return LatteCore.Types.TopEdgeLeftAlign;
            case PlasmaCore.Types.LeftEdge: return LatteCore.Types.LeftEdgeTopAlign;
            case PlasmaCore.Types.RightEdge: return LatteCore.Types.RightEdgeTopAlign;
            }

            return LatteCore.Types.BottomEdgeLeftAlign;
        }
    }

    AppletsContainer {
        id: _mainLayout
        beginIndex: 100
        offset: centered ? appliedOffset : root.offsetFixed

        readonly property bool centered: (root.panelAlignment === LatteCore.Types.Center) || (root.panelAlignment === LatteCore.Types.Justify)
        readonly property bool reversed: Qt.application.layoutDirection === Qt.RightToLeft
        readonly property int appliedOffset: root.panelAlignment === LatteCore.Types.Justify ? 0 : root.offset

        alignment: {
            if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                if (centered) return LatteCore.Types.LeftEdgeCenterAlign;
                if (root.panelAlignment === LatteCore.Types.Top) return LatteCore.Types.LeftEdgeTopAlign;
                if (root.panelAlignment === LatteCore.Types.Bottom) return LatteCore.Types.LeftEdgeBottomAlign;
            }

            if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                if (centered) return LatteCore.Types.RightEdgeCenterAlign;
                if (root.panelAlignment === LatteCore.Types.Top) return LatteCore.Types.RightEdgeTopAlign;
                if (root.panelAlignment === LatteCore.Types.Bottom) return LatteCore.Types.RightEdgeBottomAlign;
            }

            if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                if (centered) return LatteCore.Types.BottomEdgeCenterAlign;

                if ((root.panelAlignment === LatteCore.Types.Left && !reversed)
                        || (root.panelAlignment === LatteCore.Types.Right && reversed)) {
                    return LatteCore.Types.BottomEdgeLeftAlign;
                }

                if ((root.panelAlignment === LatteCore.Types.Right && !reversed)
                        || (root.panelAlignment === LatteCore.Types.Left && reversed)) {
                    return LatteCore.Types.BottomEdgeRightAlign;
                }
            }

            if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                if (centered) return LatteCore.Types.TopEdgeCenterAlign;

                if ((root.panelAlignment === LatteCore.Types.Left && !reversed)
                        || (root.panelAlignment === LatteCore.Types.Right && reversed)) {
                    return LatteCore.Types.TopEdgeLeftAlign;
                }

                if ((root.panelAlignment === LatteCore.Types.Right && !reversed)
                        || (root.panelAlignment === LatteCore.Types.Left && reversed)) {
                    return LatteCore.Types.TopEdgeRightAlign;
                }
            }

            return LatteCore.Types.BottomEdgeCenterAlign;
        }

        transitions: Transition {
            enabled: editModeVisual.plasmaEditMode
            AnchorAnimation {
                duration: 0.8 * animations.duration.proposed
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
            case PlasmaCore.Types.BottomEdge: return LatteCore.Types.BottomEdgeRightAlign;
            case PlasmaCore.Types.TopEdge: return LatteCore.Types.TopEdgeRightAlign;
            case PlasmaCore.Types.LeftEdge: return LatteCore.Types.LeftEdgeBottomAlign;
            case PlasmaCore.Types.RightEdge: return LatteCore.Types.RightEdgeBottomAlign;
            }

            return LatteCore.Types.BottomEdgeLeftAlign;
        }
    }

    function updateSizeForAppletsInFill() {
        if (!updateSizeForAppletsInFillTimer.running) {
            updateSizeForAppletsInFillTimer.start();
        }
    }

    Connections {
        target: metrics
        onIconSizeAnimationEnded: delayUpdateMaskArea.start();
    }

    //! This timer is needed in order to reduce the calls to heavy cpu function
    //! HeuristicTools.updateSizeForAppletsInFill()
    Timer{
        id: updateSizeForAppletsInFillTimer
        interval: 10
        onTriggered: HeuristicTools.updateSizeForAppletsInFill();
    }

    //! This timer is needed in order to update mask area after ContentsWidth/Height and iconSize changes
    Timer{
        id:delayUpdateMaskArea
        repeat:false;
        interval:300;

        onTriggered: {
            if (layoutsContainer.animationSent) {
                animations.needLength.removeEvent(layoutsContainer);
                layoutsContainer.animationSent = false;
            }

            visibilityManager.updateMaskArea();

            if (root.debugModeTimers) {
                console.log("LayoutsContainer timer: delayUpdateMaskArea called...");
            }
        }
    }
}
