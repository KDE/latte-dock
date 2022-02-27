/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.8

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.latte.core 0.2 as LatteCore

Item {
    id: sizer

    // when there are only plasma style task managers OR any applets that fill width or height
    // the automatic icon size algorithm should better be disabled
    readonly property bool isActive: root.behaveAsDockWithMask
                                     && plasmoid.configuration.autoSizeEnabled
                                     && !root.containsOnlyPlasmaTasks
                                     && layouter.fillApplets<=0
                                     && !(root.inConfigureAppletsMode && plasmoid.configuration.alignment === LatteCore.Types.Justify) /*block shrinking for justify splitters*/
                                     && latteView
                                     && latteView.visibility.mode !== LatteCore.Types.SidebarOnDemand
                                     && latteView.visibility.mode !== LatteCore.Types.SidebarAutoHide

    property int iconSize: -1 //it is not set, this is the default

    readonly property bool inCalculatedIconSize: ((metrics.iconSize === sizer.iconSize) || (metrics.iconSize === metrics.maxIconSize))
    readonly property bool inAutoSizeAnimation: !inCalculatedIconSize

    readonly property int automaticStep: 8
    readonly property int historyMaxSize: 10
    readonly property int historyMinSize: 4


    //! Prediction History of the algorithm in order to track cases where the algorithm produces
    //! grows and shrinks endlessly
    property variant history: []

    //! required elements
    property Item layouts
    property Item layouter
    property Item metrics
    property Item visibility

    onInAutoSizeAnimationChanged: {
        if (inAutoSizeAnimation) {
            animations.needBothAxis.addEvent(sizer);
        } else {
            animations.needBothAxis.removeEvent(sizer);
        }
    }

    onIsActiveChanged: {
        clearHistory();
        updateIconSize();
    }

    Connections {
        target: root
        onContainsOnlyPlasmaTasksChanged: sizer.updateIconSize();
        onMaxLengthChanged: {
            if (latteView && latteView.positioner && !latteView.positioner.isOffScreen) {
                sizer.updateIconSize();
            }
        }
    }

    Connections {
        target: metrics

        onPortionIconSizeChanged: {
            if (metrics.portionIconSize!==-1) {
                sizer.updateIconSize();
            }
        }
    }

    Connections {
        target: latteView
        onWidthChanged:{
            if (root.isHorizontal && metrics.portionIconSize!==-1) {
                sizer.updateIconSize();
            }
        }

        onHeightChanged:{
            if (root.isVertical && metrics.portionIconSize!==-1) {
                sizer.updateIconSize();
            }
        }
    }

    Connections {
        target: latteView && latteView.positioner ? latteView.positioner : null
        onIsOffScreenChanged: {
            if (!latteView.positioner.isOffScreen) {
                sizer.updateIconSize();
            }
        }
    }

    Connections {
        target: visibilityManager
        onInNormalStateChanged: {
            if (visibilityManager.inNormalState) {
                sizer.updateIconSize();
            }
        }
    }

    //! Prediction History Functions
    function clearHistory() {
        history.length = 0;
    }

    function addPrediction(currentLength, prediction) {
        history.unshift({current: currentLength, predicted: prediction});

        /* console.log(" -- PREDICTION ARRAY -- ");
        for(var i=0; i<history.length; ++i) {
            console.log( i + ". " + history[i].current + " : " + history[i].predicted);
        }*/

        if (history.length > historyMaxSize) {
            history.splice(historyMinSize, history.length - historyMinSize);
        }
    }

    function producesEndlessLoop(currentLength, prediction) {
        if (history.length < 2) {
            return false;
        }

        if (history[1].current === currentLength
                && history[1].predicted === prediction) {
            //! the current prediction is the same like two steps before in prediction history

            if(history[0].current > history[0].predicted && history[1].current < history[1].predicted) {
                //! case2: the algorithm that is trying to SHRINK has already produced same results subsequently
                console.log(" AUTOMATIC ITEM SIZE PROTECTOR, :: ENDLESS AUTOMATIC SIZE LOOP DETECTED");
                return true;
            }
        }

        return false;
    }


    function updateIconSize() {
        if (!isActive && iconSize !== -1) {
            // restore original icon size
            iconSize = -1;
        }

        if ( !doubleCallAutomaticUpdateIconSize.running && !visibility.inRelocationHiding /*block too many calls and dont apply during relocatinon hiding*/
                && (visibility.inNormalState && sizer.isActive) /*in normal and auto size active state*/
                && (metrics.iconSize===metrics.maxIconSize || metrics.iconSize === sizer.iconSize) /*not during animations*/) {

            //!doubler timer
            if (!doubleCallAutomaticUpdateIconSize.secondTimeCallApplied) {
                doubleCallAutomaticUpdateIconSize.start();
            } else {
                doubleCallAutomaticUpdateIconSize.secondTimeCallApplied = false;
            }

            var layoutLength;
            var maxLength = root.maxLength;

            //console.log("------Entered check-----");
            //console.log("max length: "+ maxLength);

            layoutLength = (plasmoid.configuration.alignment === LatteCore.Types.Justify) ?
                        layouts.startLayout.length+layouts.mainLayout.length+layouts.endLayout.length : layouts.mainLayout.length


            var itemLength = metrics.totals.length;

            var toShrinkLimit = maxLength - (parabolic.factor.zoom * itemLength);
            //! to grow limit must be a little less than the shrink one in order to be more robust and
            //! not create endless loops from early calculations
            var toGrowLimit = maxLength - (1.2 * parabolic.factor.zoom * itemLength);

            //console.log("toShrinkLimit: "+ toShrinkLimit);
            //console.log("toGrowLimit: "+ toGrowLimit);

            var newIconSizeFound = false;
            if (layoutLength > toShrinkLimit) { //must shrink
                // console.log("step3");
                var nextIconSize = metrics.maxIconSize;

                do {
                    nextIconSize = nextIconSize - automaticStep;
                    var factor = nextIconSize / metrics.iconSize;
                    var nextLength = factor * layoutLength;

                } while ( (nextLength>toShrinkLimit) && (nextIconSize !== 16));

                var intLength = Math.round(layoutLength);
                var intNextLength = Math.round(nextLength);

                iconSize = nextIconSize;
                newIconSizeFound = true;

                addPrediction(intLength, intNextLength);
                // console.log("Step 3 - found:"+iconSize);
            } else if ((layoutLength<toGrowLimit
                        && (metrics.iconSize === iconSize)) ) { //must grow probably
                // console.log("step4");
                var nextIconSize2 = iconSize;
                var foundGoodSize = -1;

                do {
                    nextIconSize2 = nextIconSize2 + automaticStep;
                    var factor2 = nextIconSize2 / iconSize;
                    var nextLength2 = factor2 * layoutLength;

                    if (nextLength2 < toGrowLimit) {
                        foundGoodSize = nextIconSize2;
                    }
                } while ( (nextLength2<toGrowLimit) && (nextIconSize2 !== metrics.maxIconSize ));

                var intLength2 = Math.round(layoutLength);
                var intNextLength2 = Math.round(nextLength2);

                if (foundGoodSize > 0 && !producesEndlessLoop(intLength2, intNextLength2)) {
                    if (foundGoodSize === metrics.maxIconSize) {
                        iconSize = -1;
                    } else {
                        iconSize = foundGoodSize;
                    }
                    newIconSizeFound = true

                    addPrediction(intLength2, intNextLength2);
                    //        console.log("Step 4 - found:"+iconSize);
                } else {
                    //       console.log("Step 4 - did not found...");
                }
            }
        }
    }

    //! This functions makes sure to call the updateIconSize(); function which is costly
    //! one more time after its last call to confirm the applied icon size found
    Timer{
        id:doubleCallAutomaticUpdateIconSize
        interval: 1000
        property bool secondTimeCallApplied: false

        onTriggered: {
            if (!secondTimeCallApplied) {
                secondTimeCallApplied = true;
                sizer.updateIconSize();
            }
        }
    }
}
