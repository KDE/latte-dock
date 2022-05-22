/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.private.app 0.1 as LatteApp
import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.private.containment 0.1 as LatteContainment

import "../debugger" as Debugger

Item{
    id: layoutsContainer
    //! WorkAround: Do not use "visible" because when it becomes "false" the contained applets can hide/show their elements.
    //! That approach can create a conflict with Latte Tasks that after showing the view they reshow windows
    //! that were already shown before hiding.
    //! visible: !(latteView && latteView.visibility.isHidden)
    opacity: !(latteView && latteView.visibility.isHidden) ? 1 : 0

    readonly property bool isHidden: root.inStartup || (latteView && latteView.visibility && latteView.visibility.isHidden)

    property int currentSpot: -1000

    readonly property alias startLayout : _startLayout
    readonly property alias mainLayout: _mainLayout
    readonly property alias endLayout: _endLayout
    readonly property alias contextMenuIsShown: contextMenuLayer.menuIsShown

    signal contentsLengthChanged();

    Binding {
        target: layoutsContainer
        property: "x"
        when: !visibilityManager.inRelocationAnimation
        value: {
            if (root.behaveAsPlasmaPanel) {
                return 0;
            }

            if ( latteView && root.isHorizontal && root.myView.alignment === LatteCore.Types.Justify ){
                return ((latteView.width/2) - (root.maxLength/2) + background.offset);
            } else {
                if ((root.myView.inSlidingIn || root.myView.inSlidingOut) && root.isVertical){
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
        when: !visibilityManager.inRelocationAnimation
        value: {
            if (root.behaveAsPlasmaPanel) {
                return 0;
            }

            if ( latteView && root.isVertical && root.myView.alignment === LatteCore.Types.Justify ) {
                return ((latteView.height/2) - (root.maxLength/2) + background.offset);
            } else {
                if ((root.myView.inSlidingIn || root.myView.inSlidingOut) && root.isHorizontal){
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

    width: root.isHorizontal && root.myView.alignment === LatteCore.Types.Justify ? root.maxLength : parent.width
    height: root.isVertical && root.myView.alignment === LatteCore.Types.Justify ? root.maxLength : parent.height
    z:10

    property bool animationSent: false
    property bool shouldCheckHalfs: (plasmoid.configuration.alignment === LatteCore.Types.Justify) && (_mainLayout.children>1)

    property int contentsWidth: root.isHorizontal ? _startLayout.width + _mainLayout.width + _endLayout.width :
                                                    Math.max(_startLayout.width, _mainLayout.width ,_endLayout.width)
    property int contentsHeight: root.isVertical ? _startLayout.height + _mainLayout.height + _endLayout.height :
                                                   Math.max(_startLayout.height, _mainLayout.height, _endLayout.height)


    readonly property int backgroundShadowTailLength: {
        if (root.behaveAsPlasmaPanel) {
            return 0;
        }

        if (root.myView.alignment === LatteCore.Types.Left) {
            return background.shadows.left;
        } else if (root.myView.alignment === LatteCore.Types.Right) {
            return background.shadows.right;
        } else if (root.myView.alignment === LatteCore.Types.Top) {
            return background.shadows.top;
        } else if (root.myView.alignment === LatteCore.Types.Bottom) {
            return background.shadows.bottom;
        }

        //! centered case
        return root.isHorizontal ? background.shadows.left : background.shadows.top;
    }

    readonly property int backgroundShadowHeadLength: {
        if (root.behaveAsPlasmaPanel) {
            return 0;
        }

        if (root.myView.alignment === LatteCore.Types.Left) {
            return background.shadows.right;
        } else if (root.myView.alignment === LatteCore.Types.Right) {
            return background.shadows.left;
        } else if (root.myView.alignment === LatteCore.Types.Top) {
            return background.shadows.bottom;
        } else if (root.myView.alignment === LatteCore.Types.Bottom) {
            return background.shadows.top;
        }

        //! centered case
        return root.isHorizontal ? background.shadows.right : background.shadows.bottom;
    }

    readonly property int lengthTailPadding: {
        var minimumPadding = metrics.margin.length;
        var bestMatchingPadding = 0;

        if (root.myView.alignment === LatteCore.Types.Left) {
            bestMatchingPadding = Math.max(background.paddings.left, minimumPadding) - minimumPadding;
        } else if (root.myView.alignment === LatteCore.Types.Right) {
            bestMatchingPadding = Math.max(background.paddings.right, minimumPadding) - minimumPadding;
        } else if (root.myView.alignment === LatteCore.Types.Top) {
            bestMatchingPadding = Math.max(background.paddings.top, minimumPadding) - minimumPadding;
        } else if (root.myView.alignment === LatteCore.Types.Bottom) {
            bestMatchingPadding = Math.max(background.paddings.bottom, minimumPadding) - minimumPadding;
        } else if (root.myView.alignment === LatteCore.Types.Center) {
            bestMatchingPadding = Math.max(background.paddings.left, minimumPadding) - minimumPadding;
        } else if (root.myView.alignment === LatteCore.Types.Justify) {
            var backpadding = root.isHorizontal ? background.paddings.left : background.paddings.top;
            bestMatchingPadding = Math.max(backpadding, minimumPadding) - minimumPadding;
        }

        //shadow is already calculated in Justify mode
        return root.myView.alignment !== LatteCore.Types.Justify ? backgroundShadowTailLength + bestMatchingPadding : bestMatchingPadding;
    }

    readonly property int lengthHeadPadding: {
        var minimumPadding = metrics.margin.length;
        var bestMatchingPadding = 0;

        if (root.myView.alignment === LatteCore.Types.Left) {
            bestMatchingPadding = Math.max(background.paddings.right, minimumPadding) - minimumPadding;
        } else if (root.myView.alignment === LatteCore.Types.Right) {
            bestMatchingPadding = Math.max(background.paddings.left, minimumPadding) - minimumPadding;
        } else if (root.myView.alignment === LatteCore.Types.Top) {
            bestMatchingPadding = Math.max(background.paddings.bottom, minimumPadding) - minimumPadding;
        } else if (root.myView.alignment === LatteCore.Types.Bottom) {
            bestMatchingPadding = Math.max(background.paddings.top, minimumPadding) - minimumPadding;
        } else if (root.myView.alignment === LatteCore.Types.Center) {
            bestMatchingPadding = Math.max(background.paddings.right, minimumPadding) - minimumPadding;
        } else if (root.myView.alignment === LatteCore.Types.Justify) {
            var backpadding = root.isHorizontal ? background.paddings.right : background.paddings.bottom;
            bestMatchingPadding = Math.max(backpadding, minimumPadding) - minimumPadding;
        }

        //shadow is already calculated in Justify mode
        return root.myView.alignment !== LatteCore.Types.Justify ? backgroundShadowHeadLength + bestMatchingPadding : bestMatchingPadding;
    }

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

            contentsLengthChanged();

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

            contentsLengthChanged();

            delayUpdateMaskArea.start();
        }
    }

    onXChanged: root.updateEffectsArea();
    onYChanged: root.updateEffectsArea();

    EnvironmentActions {
        id: environmentActions
        active: root.scrollAction !== LatteContainment.Types.ScrollNone || root.dragActiveWindowEnabled || root.closeActiveWindowEnabled
        alignment: _mainLayout.alignment
    }

    LatteApp.ContextMenuLayer {
        id: contextMenuLayer
        anchors.fill: parent
        view: latteView
    }

    AppletsContainer {
        id: _startLayout
        beginIndex: 0
        offset: lengthTailPadding
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

    /*Rectangle {
        anchors.fill: _mainLayout
        color: "transparent"
        border.width: 1
        border.color: "black"
    }*/

    AppletsContainer {
        id: _mainLayout
        z:10 //be on top of start and end layouts
        beginIndex: 100
        offset: {
            if (!centered) {
                //! it is used for Top/Bottom/Left/Right alignments when they show both background length shadows
                return background.offset + lengthTailPadding;
            }

            return (root.myView.alignment === LatteCore.Types.Justify) ? inJustifyCenterOffset : background.offset - parabolicOffsetting
        }

        ignoredLength: startParabolicSpacer.length + endParabolicSpacer.length

        readonly property alias startParabolicSpacer: _startParabolicSpacer
        readonly property alias endParabolicSpacer: _endParabolicSpacer

        readonly property bool centered: (root.myView.alignment === LatteCore.Types.Center) || (root.myView.alignment === LatteCore.Types.Justify)
        readonly property bool reversed: Qt.application.layoutDirection === Qt.RightToLeft
        readonly property real parabolicOffsetting: (startParabolicSpacer.length - endParabolicSpacer.length) / 2
        property int inJustifyCenterOffset: 0

        alignment: {
            if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                if (centered) return LatteCore.Types.LeftEdgeCenterAlign;
                if (root.myView.alignment === LatteCore.Types.Top) return LatteCore.Types.LeftEdgeTopAlign;
                if (root.myView.alignment === LatteCore.Types.Bottom) return LatteCore.Types.LeftEdgeBottomAlign;
            }

            if (plasmoid.location === PlasmaCore.Types.RightEdge) {
                if (centered) return LatteCore.Types.RightEdgeCenterAlign;
                if (root.myView.alignment === LatteCore.Types.Top) return LatteCore.Types.RightEdgeTopAlign;
                if (root.myView.alignment === LatteCore.Types.Bottom) return LatteCore.Types.RightEdgeBottomAlign;
            }

            if (plasmoid.location === PlasmaCore.Types.BottomEdge) {
                if (centered) return LatteCore.Types.BottomEdgeCenterAlign;

                if ((root.myView.alignment === LatteCore.Types.Left && !reversed)
                        || (root.myView.alignment === LatteCore.Types.Right && reversed)) {
                    return LatteCore.Types.BottomEdgeLeftAlign;
                }

                if ((root.myView.alignment === LatteCore.Types.Right && !reversed)
                        || (root.myView.alignment === LatteCore.Types.Left && reversed)) {
                    return LatteCore.Types.BottomEdgeRightAlign;
                }
            }

            if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                if (centered) return LatteCore.Types.TopEdgeCenterAlign;

                if ((root.myView.alignment === LatteCore.Types.Left && !reversed)
                        || (root.myView.alignment === LatteCore.Types.Right && reversed)) {
                    return LatteCore.Types.TopEdgeLeftAlign;
                }

                if ((root.myView.alignment === LatteCore.Types.Right && !reversed)
                        || (root.myView.alignment === LatteCore.Types.Left && reversed)) {
                    return LatteCore.Types.TopEdgeRightAlign;
                }
            }

            return LatteCore.Types.BottomEdgeCenterAlign;
        }

        transitions: Transition {
            enabled: !visibilityManager.inRelocationAnimation && !root.inStartup
            AnchorAnimation {
                duration: 0.8 * animations.duration.proposed
                easing.type: Easing.OutCubic
            }
        }

        ParabolicEdgeSpacer {
            id: _startParabolicSpacer
            index: mainLayout.beginIndex - 1
        }

        ParabolicEdgeSpacer {
            id: _endParabolicSpacer
            index: mainLayout.beginIndex + mainLayout.children.length - 2
        }

        Binding{
            target: _mainLayout
            property:"inJustifyCenterOffset"
            when: !layouter.appletsInParentChange && layouter.inNormalFillCalculationsState
            value: {
                if (root.myView.alignment !== LatteCore.Types.Justify) {
                    return 0;
                }

                var layoutMaxLength = root.maxLength / 2;
                var sideLayoutMaxLength = layoutMaxLength - mainLayout.length/2;
                var sideslength = startLayout.length + endLayout.length;

                if (sideslength > root.maxLength) {
                    return 0;
                }

                if (startLayout.length > sideLayoutMaxLength) {
                    return (startLayout.length - sideLayoutMaxLength);
                } else if (endLayout.length > sideLayoutMaxLength) {
                    return -(endLayout.length - sideLayoutMaxLength);
                }

                return 0;
            }
        }
    }

    AppletsContainer {
        id: _endLayout
        beginIndex: 200
        offset: lengthHeadPadding
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

    Connections {
        target: metrics
        onIconSizeAnimationEnded: delayUpdateMaskArea.start();
    }

    //! Debug Elements
    Loader{
        anchors.top: startLayout.top
        anchors.horizontalCenter: startLayout.horizontalCenter
        active: debug.layouterEnabled

        readonly property Item debugLayout: layouter.startLayout

        sourceComponent: Debugger.Tag{
            background.color: "white"
            label.text: tagText
            label.color: "black"
            label.font.pointSize: 13
            readonly property int layoutLength: root.isHorizontal ? debugLayout.grid.width : debugLayout.grid.height

            readonly property string tagText: {
                return "normal:" + debugLayout.shownApplets + " / fill:" + debugLayout.fillApplets + " / reg_len:" + debugLayout.sizeWithNoFillApplets + " / tot_len:"+layoutLength;
            }
        }
    }

    Loader{
        anchors.top: endLayout.top
        anchors.horizontalCenter: endLayout.horizontalCenter
        active: debug.layouterEnabled

        readonly property Item debugLayout: layouter.endLayout

        sourceComponent: Debugger.Tag{
            background.color: "white"
            label.text: tagText
            label.color: "black"
            label.font.pointSize: 13
            readonly property int layoutLength: root.isHorizontal ? debugLayout.grid.width : debugLayout.grid.height

            readonly property string tagText: {
                return "normal:" + debugLayout.shownApplets + " / fill:" + debugLayout.fillApplets + " / reg_len:" + debugLayout.sizeWithNoFillApplets + " / tot_len:"+layoutLength;
            }
        }
    }

    Loader{
        anchors.top: mainLayout.top
        anchors.horizontalCenter: mainLayout.horizontalCenter
        active: debug.layouterEnabled
        z:70

        readonly property Item debugLayout: layouter.mainLayout

        sourceComponent: Debugger.Tag{
            background.color: "white"
            label.text: tagText
            label.color: "black"
            label.font.pointSize: 13
            readonly property int layoutLength: root.isHorizontal ? debugLayout.grid.width : debugLayout.grid.height

            readonly property string tagText: {
                return "normal:" + debugLayout.shownApplets + " / fill:" + debugLayout.fillApplets + " / reg_len:" + debugLayout.sizeWithNoFillApplets + " / tot_len:"+layoutLength;
            }
        }
    }

    Loader{
        anchors.top: mainLayout.top
        anchors.left: parent.left
        active: debug.layouterEnabled

        readonly property Item debugLayout: layoutsContainer

        sourceComponent: Debugger.Tag{
            background.color: "blue"
            label.text: tagText
            label.color: "yellow"
            label.font.pointSize: 13
            label.font.bold: true
            readonly property int layoutLength: root.isHorizontal ? debugLayout.width : debugLayout.height

            readonly property int layoutsLength: {
                if (root.isVertical) {
                    return layouter.startLayout.grid.height + layouter.mainLayout.grid.height + layouter.endLayout.grid.height;
                }

                return layouter.startLayout.grid.width + layouter.mainLayout.grid.width + layouter.endLayout.grid.width;
            }

            readonly property string tagText: {
                return "MAX:" + root.maxLength + " / MIN: " + root.minLength + " TOT:"+layoutLength + " / LAYS:"+ layoutsLength;
            }
        }
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

            if (debug.timersEnabled) {
                console.log("LayoutsContainer timer: delayUpdateMaskArea called...");
            }
        }
    }
}
