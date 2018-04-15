/*
*  Copyright 2017-2018 Michail Vourlakos <mvourlakos@gmail.com>
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

//!         FILLWIDTH/FILLHEIGHT COMPUTATIONS
//! Computations in order to calculate correctly the sizes for applets
//! that are requesting fillWidth or fillHeight


//! initialize AppletItems flag "inFillCalculations" in orderf
//! to inform them that new calculations are taking place
function initLayoutForFillsCalculations(layout) {
    for(var i=0; i<layout.children.length; ++i) {
        var curApplet = layout.children[i];
        if (curApplet.needsFillSpace) {
            curApplet.inFillCalculations = true;
        }
    }
}

//! during step1/pass1 all the applets with fills
//! that use lower maximumSize than the proposed one from the
//! algorithm are updated
function computeStep1ForLayout(layout, availableSpace, sizePerApplet, noOfApplets) {
    for(var i=0; i<layout.children.length; ++i) {
        var curApplet = layout.children[i];
        var maxSize = 0;

        if (curApplet && curApplet.applet && curApplet.applet.Layout)
            maxSize = root.isVertical ? curApplet.applet.Layout.maximumHeight : curApplet.applet.Layout.maximumWidth;

        if (curApplet.needsFillSpace && (maxSize <= sizePerApplet) && (maxSize>=1) && (maxSize !== Infinity)) {
            curApplet.sizeForFill = maxSize;
            // console.log("s3_1  "+ maxSize);
            curApplet.inFillCalculations = false;
            availableSpace = Math.abs(availableSpace - maxSize);
            noOfApplets = noOfApplets - 1;
            sizePerApplet = noOfApplets > 1 ? Math.floor(availableSpace / noOfApplets) : availableSpace;
            // console.log(noOfApplets + " - " + sizePerApplet + " - " + availableSpace);
        }
    }

    return [availableSpace, sizePerApplet, noOfApplets];
}

//! during step2/pass2 all the applets with fills
//! that remained with no computations from pass1
//! are updated with the algorithm's proposed size
function computeStep2ForLayout(layout, sizePerApplet) {
    for(var i=0; i<layout.children.length; ++i) {
        var curApplet = layout.children[i];

        if (curApplet.needsFillSpace && curApplet.inFillCalculations) {
            curApplet.sizeForFill = sizePerApplet;
            // console.log("s4_1  "+ sizePerApplet);
            curApplet.inFillCalculations = false;
        }
    }
}

//! initialize the three layouts and execute the step1/phase1
//! it is used when the Centered (Main)Layout is only used or when the Main(Layout)
//! is empty in Justify mode
function initializationPhase(availableSpace, sizePerApplet, noOfApplets){
    if (root.panelAlignment === Latte.Dock.Justify) {
        initLayoutForFillsCalculations(startLayout);
        initLayoutForFillsCalculations(endLayout);
    }
    initLayoutForFillsCalculations(mainLayout);


    // console.log("s3...");

    //! first pass in order to update sizes for applet that want to fill space
    //! but their maximum metrics are lower than the sizePerApplet
    var res = computeStep1ForLayout(mainLayout, availableSpace, sizePerApplet, noOfApplets);
    availableSpace = res[0]; sizePerApplet = res[1]; noOfApplets = res[2];

    if (root.panelAlignment === Latte.Dock.Justify) {
        res = computeStep1ForLayout(startLayout, availableSpace, sizePerApplet, noOfApplets);
        availableSpace = res[0]; sizePerApplet = res[1]; noOfApplets = res[2];

        res = computeStep1ForLayout(endLayout, availableSpace, sizePerApplet, noOfApplets);
        availableSpace = res[0]; sizePerApplet = res[1]; noOfApplets = res[2];
    }

    return [availableSpace, sizePerApplet, noOfApplets];
}


function updateSizeForAppletsInFill() {
    if ((visibilityManager.normalState && !root.editMode)
            || (behaveAsPlasmaPanel && root.editMode)) {
        // console.log("-------------");
        // console.log("s1...");
        var noA = startLayout.fillApplets + mainLayout.fillApplets + endLayout.fillApplets;

        if (noA === 0)
            return;

        // console.log("s2...");
        if (mainLayout.shownApplets === 0 || root.panelAlignment !== Latte.Dock.Justify) {
            var availableSpace = Math.max(0, root.maxLength - startLayout.sizeWithNoFillApplets - mainLayout.sizeWithNoFillApplets - endLayout.sizeWithNoFillApplets - root.panelEdgeSpacing);
            var sizePerApplet = availableSpace / noA;


            var res = initializationPhase(availableSpace, sizePerApplet, noA);
            availableSpace = res[0];  sizePerApplet = res[1]; noA = res[2];

            // console.log("s4...");

            //! second pass in order to update sizes for applet that want to fill space
            //! this applets get the direct division of the available free space that
            //! remained from step1
            computeStep2ForLayout(startLayout, sizePerApplet);
            computeStep2ForLayout(mainLayout, sizePerApplet);
            computeStep2ForLayout(endLayout, sizePerApplet);

            //console.log("s5...");
        } else {
            //! Justify mode in all remaining cases

            //! compute the two free spaces around the centered layout
            //! they are called start and end accordingly
            var availableSpaceStart = Math.max(0, root.maxLength/2 - startLayout.sizeWithNoFillApplets - root.panelEdgeSpacing/2);
            var availableSpaceEnd = Math.max(0, root.maxLength/2 - endLayout.sizeWithNoFillApplets - root.panelEdgeSpacing/2);
            var availableSpace = availableSpaceStart + availableSpaceEnd - mainLayout.sizeWithNoFillApplets;

            var sizePerAppletMain = mainLayout.fillApplets > 0 ? availableSpace / noA : 0 ;

            var noStart = startLayout.fillApplets;
            var noMain = mainLayout.fillApplets;
            var noEnd = endLayout.fillApplets;

            //! initialize the computations
            initLayoutForFillsCalculations(startLayout);
            initLayoutForFillsCalculations(mainLayout);
            initLayoutForFillsCalculations(endLayout);

            //console.log("s3...");
            var res;

            //! first pass
            if (mainLayout.fillApplets > 0){
                res = computeStep1ForLayout(mainLayout, availableSpace, sizePerAppletMain, noMain);
                sizePerAppletMain = res[1]; noMain = res[2];
                var dif = (availableSpace - res[0]) / 2;
                availableSpaceStart = availableSpaceStart - dif;
                availableSpaceEnd = availableSpaceEnd - dif;
            }

            var sizePerAppletStart = startLayout.fillApplets > 0 ? availableSpaceStart / noStart : 0 ;
            var sizePerAppletEnd = endLayout.fillApplets > 0 ? availableSpaceEnd / noEnd : 0 ;

            if (startLayout.fillApplets > 0) {
                res = computeStep1ForLayout(startLayout, availableSpaceStart, sizePerAppletStart, noStart);
                availableSpaceStart = res[0]; sizePerAppletStart = res[1]; noStart = res[2];
            }
            if (endLayout.fillApplets > 0) {
                res = computeStep1ForLayout(endLayout, availableSpaceEnd, sizePerAppletEnd, noEnd);
                availableSpaceEnd = res[0]; sizePerAppletEnd = res[1]; noEnd = res[2];
            }

            ////
            //! second pass
            if (mainLayout.shownApplets > 0) {
                //var bSize = root.isHorizontal ? mainLayout.width : mainLayout.height;
                sizePerAppletMain = (availableSpaceStart + availableSpaceEnd) / (noStart + noMain + noEnd);
                computeStep2ForLayout(mainLayout, sizePerAppletMain);
                var aSize = root.isHorizontal ? mainLayout.width : mainLayout.height;

                availableSpaceStart = availableSpaceStart - aSize/2;
                availableSpaceEnd = availableSpaceEnd - aSize/2;

                sizePerAppletStart = startLayout.fillApplets > 0 ? availableSpaceStart / noStart : 0 ;
                sizePerAppletEnd = endLayout.fillApplets > 0 ? availableSpaceEnd / noEnd : 0 ;
            }

            if (startLayout.fillApplets > 0)
                computeStep2ForLayout(startLayout, sizePerAppletStart);
            if (endLayout.fillApplets > 0)
                computeStep2ForLayout(endLayout, sizePerAppletEnd);
        }
    }
}
