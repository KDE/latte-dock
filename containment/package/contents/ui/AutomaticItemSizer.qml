/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.8

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0

import org.kde.latte 0.2 as Latte

Item {
    id: sizer

    // when there are only plasma style task managers OR any applets that fill width or height
    // the automatic icon size algorithm should better be disabled
    readonly property bool isActive: !root.containsOnlyPlasmaTasks && layoutsContainer.fillApplets<=0

    property bool automaticSizeAnimation: false

    readonly property int automaticStep: 8
    readonly property int historyMaxSize: 10
    readonly property int historyMinSize: 4
    property int automaticIconSizeBasedSize: -1 //it is not set, this is the default

    //! Prediction History of the algorithm in order to track cases where the algorithm produces
    //! grows and shrinks endlessly
    property variant history: []

    onAutomaticIconSizeBasedSizeChanged: {
        if (!automaticSizeAnimation) {
            automaticSizeAnimation = true;
            root.slotAnimationsNeedBothAxis(1);
        }
    }

    onIsActiveChanged: clearHistory();

    Connections {
        target: root

        onContainsOnlyPlasmaTasksChanged: sizer.updateAutomaticIconSize();
        onEditModeChanged: sizer.updateAutomaticIconSize();
        onMaxLengthChanged: {
            if (root.editMode) {
                sizer.updateAutomaticIconSize();
            }
        }
        onProportionIconSizeChanged: {
            if (root.proportionIconSize!==-1) {
                sizer.updateAutomaticIconSize();
            }
        }

        onIconSizeChanged: {
            if (((root.iconSize === sizer.automaticIconSizeBasedSize) || (root.iconSize === root.maxIconSize)) && sizer.automaticSizeAnimation){
                root.slotAnimationsNeedBothAxis(-1);
                sizer.automaticSizeAnimation=false;
            }
        }
    }

    Connections {
        target: latteView
        onWidthChanged:{
            if (root.isHorizontal && root.proportionIconSize!==-1) {
                sizer.updateAutomaticIconSize();
            }
        }

        onHeightChanged:{
            if (root.isVertical && root.proportionIconSize!==-1) {
                sizer.updateAutomaticIconSize();
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


    function updateAutomaticIconSize() {
        if ( !doubleCallAutomaticUpdateIconSize.running && !visibilityManager.inTempHiding
                && ((visibilityManager.normalState || root.editMode)
                    && (sizer.isActive || (!sizer.isActive && root.iconSize!==root.maxIconSize)))
                && (root.iconSize===root.maxIconSize || root.iconSize === sizer.automaticIconSizeBasedSize) ) {

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

            if (root.isVertical) {
                layoutLength = (plasmoid.configuration.panelPosition === Latte.Types.Justify) ?
                            layoutsContainer.startLayout.height+layoutsContainer.mainLayout.height+layoutsContainer.endLayout.height : layoutsContainer.mainLayout.height
            } else {
                layoutLength = (plasmoid.configuration.panelPosition === Latte.Types.Justify) ?
                            layoutsContainer.startLayout.width+layoutsContainer.mainLayout.width+layoutsContainer.endLayout.width : layoutsContainer.mainLayout.width
            }

            var toShrinkLimit = maxLength-((1+(root.zoomFactor-1))*(root.iconSize + lengthMargins));
            var toGrowLimit = maxLength-((1+(root.zoomFactor-1))*(root.iconSize + lengthMargins));

            //console.log("toShrinkLimit: "+ toShrinkLimit);
            //console.log("toGrowLimit: "+ toGrowLimit);

            var newIconSizeFound = false;
            if (layoutLength > toShrinkLimit) { //must shrink
                // console.log("step3");
                var nextIconSize = root.maxIconSize;

                do {
                    nextIconSize = nextIconSize - automaticStep;
                    var factor = nextIconSize / root.iconSize;
                    var nextLength = factor * layoutLength;

                } while ( (nextLength>toShrinkLimit) && (nextIconSize !== 16));

                var intLength = Math.round(layoutLength);
                var intNextLength = Math.round(nextLength);

                automaticIconSizeBasedSize = nextIconSize;
                newIconSizeFound = true;

                addPrediction(intLength, intNextLength);
                // console.log("Step 3 - found:"+automaticIconSizeBasedSize);
            } else if ((layoutLength<toGrowLimit
                        && (root.iconSize === automaticIconSizeBasedSize)) ) { //must grow probably
                // console.log("step4");
                var nextIconSize2 = automaticIconSizeBasedSize;
                var foundGoodSize = -1;

                do {
                    nextIconSize2 = nextIconSize2 + automaticStep;
                    var factor2 = nextIconSize2 / automaticIconSizeBasedSize;
                    var nextLength2 = factor2 * layoutLength;

                    if (nextLength2 < toGrowLimit) {
                        foundGoodSize = nextIconSize2;
                    }
                } while ( (nextLength2<toGrowLimit) && (nextIconSize2 !== root.maxIconSize ));

                var intLength2 = Math.round(layoutLength);
                var intNextLength2 = Math.round(nextLength2);

                if (foundGoodSize > 0 && !producesEndlessLoop(intLength2, intNextLength2)) {
                    if (foundGoodSize === root.maxIconSize) {
                        automaticIconSizeBasedSize = -1;
                    } else {
                        automaticIconSizeBasedSize = foundGoodSize;
                    }
                    newIconSizeFound = true

                    addPrediction(intLength2, intNextLength2);
                    //        console.log("Step 4 - found:"+automaticIconSizeBasedSize);
                } else {
                    //       console.log("Step 4 - did not found...");
                }
            }
        }
    }

    //! This functions makes sure to call the updateAutomaticIconSize(); function which is costly
    //! one more time after its last call to confirm the applied icon size found
    Timer{
        id:doubleCallAutomaticUpdateIconSize
        interval: 1000
        property bool secondTimeCallApplied: false

        onTriggered: {
            if (!secondTimeCallApplied) {
                secondTimeCallApplied = true;
                sizer.updateAutomaticIconSize();
            }
        }
    }
}
