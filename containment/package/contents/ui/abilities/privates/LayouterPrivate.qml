/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
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

import QtQuick 2.7
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore

import "./layouter" as LayouterElements

Item {
    property Item layouts: null
    property Item animations: null
    property Item indexer: null

    readonly property int fillApplets: startLayout.fillApplets + mainLayout.fillApplets + endLayout.fillApplets

    readonly property int maxLength: root.panelAlignment === LatteCore.Types.Justify ? root.maxLength : Math.min(root.minLength, root.maxLength)

    readonly property Item startLayout: LayouterElements.AppletsContainer {
        grid: layouts.startLayout
    }

    readonly property Item mainLayout: LayouterElements.AppletsContainer {
        grid: layouts.mainLayout
    }

    readonly property Item endLayout: LayouterElements.AppletsContainer {
        grid: layouts.endLayout
    }

    //!         FILLWIDTH/FILLHEIGHT COMPUTATIONS
    //! Computations in order to calculate correctly the sizes for applets
    //! that are requesting fillWidth or fillHeight

    //! qBound style function that is specialized in Layouts
    //! meaning that -1 values are ignored for fillWidth(s)/Height(s)
    function appletPreferredLength(min, pref, max){
        if (max === -1) {
            max = pref === -1 ? min : pref;
        }

        if (pref === -1) {
            pref = max === -1 ? min : pref;
        }

        return  Math.min(Math.max(min,pref),max);
    }


    //! initialize AppletItems flag "inFillCalculations" in order
    //! to inform them that new calculations are taking place
    function initLayoutForFillsCalculations(layout) {
        for(var i=0; i<layout.children.length; ++i) {
            var curApplet = layout.children[i];
            if (curApplet.isAutoFillApplet) {
                curApplet.inFillCalculations = true;
            }
        }
    }

    //! during step1/pass1 all applets that provide valid metrics (minimum/preferred/maximum values)
    //! they gain a valid space in order to draw themeselves
    function computeStep1ForLayout(layout, availableSpace, sizePerApplet, noOfApplets, inMaxAutoFillCalculations) {
        for(var i=0; i<layout.children.length; ++i) {
            var curApplet = layout.children[i];

            if (curApplet.isAutoFillApplet) {
                if (curApplet && curApplet.applet && curApplet.applet.Layout) {
                    var minSize = root.isVertical ? curApplet.applet.Layout.minimumHeight : curApplet.applet.Layout.minimumWidth;
                    var prefSize = root.isVertical ? curApplet.applet.Layout.preferredHeight : curApplet.applet.Layout.preferredWidth;
                    var maxSize = root.isVertical ? curApplet.applet.Layout.maximumHeight : curApplet.applet.Layout.maximumWidth;

                    // console.log( " s3_0 " + curApplet.applet.pluginName + " : (" +minSize+","+prefSize+","+maxSize+") ");

                    minSize = minSize>=0 && minSize!==Infinity ? minSize : -1;
                    prefSize = minSize>=0 && prefSize!==Infinity ? prefSize : -1;
                    maxSize = maxSize>=0 && maxSize!== Infinity ? maxSize : -1;

                    var appliedSize = -1;

                    //! check if the applet does not provide any valid metrics and for that case
                    //! the system must decide what space to be given after the applets that provide
                    //! nice metrics are assigned their sizes
                    var systemDecide = ((minSize<0) && (prefSize<0) && (maxSize<0));

                    if (!systemDecide) {
                        if (noOfApplets>1) {
                            appliedSize = appletPreferredLength(minSize, prefSize, maxSize);

                            // console.log( " s3_1 " + curApplet.applet.pluginName + " : (" +minSize+","+prefSize+","+maxSize+") -> " + appliedSize);
                        } else if (noOfApplets===1) {
                            //! at this step if only one applet has remained for which the max size is not null,
                            //! then for this applet we make sure the maximum size does not exceed the available space
                            //! in order for the applet to not be drawn outside the boundaries
                            appliedSize = appletPreferredLength(minSize, prefSize, Math.min(maxSize, sizePerApplet));

                            // console.log( " s3_2 " + curApplet.applet.pluginName + " : (" +minSize+","+prefSize+","+maxSize+") -> " + appliedSize);
                        }

                        //! appliedSize is valid and is also lower than the availableSpace, if it is not lower then
                        //! for this applet the needed space will be provided as a second pass in a fair way
                        //! between all remained applets that did not gain a valid fill space
                        if (appliedSize>=0 && appliedSize<=sizePerApplet) {
                            var properSize = Math.min(appliedSize, availableSpace);
                            var thickness = root.isVertical ? root.width : root.height;
                            var adjustedSize = curApplet.isHidden ? 0 : Math.max(thickness, properSize);

                            if (inMaxAutoFillCalculations) {
                                curApplet.maxAutoFillLength = adjustedSize;
                            } else {
                                curApplet.minAutoFillLength = adjustedSize;
                            }

                            curApplet.inFillCalculations = false;
                            availableSpace = Math.max(0, availableSpace - curApplet.maxAutoFillLength);
                            noOfApplets = noOfApplets - 1;
                            sizePerApplet = noOfApplets > 1 ? Math.floor(availableSpace / noOfApplets) : availableSpace;

                            // console.log( " s3_3 " + curApplet.applet.pluginName + " assigned: " + curApplet.maxAutoFillLength);
                        }
                    }

                    // console.log("s3_r " +curApplet.applet.pluginName + " : " + availableSpace + " _ " + sizePerApplet + " _ " + noOfApplets + "\n");
                }
            }
        }

        return [availableSpace, sizePerApplet, noOfApplets];
    }

    //! during step2/pass2 all the applets with fills
    //! that remained with no computations from pass1
    //! are updated with the algorithm's proposed size
    function computeStep2ForLayout(layout, sizePerApplet, noOfApplets, inMaxAutoFillCalculations) {
        if (sizePerApplet>=0) {
            if (noOfApplets === 0) {
                //! when all applets have assigned some size and there is still free space, we must find
                //! the most demanding space applet and assign the remaining space to it

                var mostDemandingAppletSize = 0;
                var mostDemandingApplet = undefined;

                //! applets with no strong opinion
                var neutralAppletsNo = 0;
                var neutralApplets = [];

                for(var i=0; i<layout.children.length; ++i) {
                    var curApplet = layout.children[i];

                    //! the most demanding applet is the one that has maximum size set to Infinity
                    //! AND is not Neutral, meaning that it provided some valid metrics
                    //! AND at the same time gained from step one the biggest space
                    if (curApplet && curApplet.isAutoFillApplet && curApplet.applet && curApplet.applet.Layout) {
                        var minSize = root.isVertical ? curApplet.applet.Layout.minimumHeight : curApplet.applet.Layout.minimumWidth;
                        var prefSize = root.isVertical ? curApplet.applet.Layout.preferredHeight : curApplet.applet.Layout.preferredWidth;
                        var maxSize = root.isVertical ? curApplet.applet.Layout.maximumHeight : curApplet.applet.Layout.maximumWidth;

                        var isNeutral = (minSize<=0 && prefSize<=0);

                        // console.log( " s4_0 " + curApplet.applet.pluginName + " : (" +minSize+","+prefSize+","+maxSize+") ");

                        if (!isNeutral && maxSize===Infinity
                                && ((inMaxAutoFillCalculations && curApplet.maxAutoFillLength>mostDemandingAppletSize)
                                    || (!inMaxAutoFillCalculations && curApplet.minAutoFillLength>mostDemandingAppletSize) )) {
                            mostDemandingApplet = curApplet;
                            mostDemandingAppletSize = inMaxAutoFillCalculations ? curApplet.maxAutoFillLength : curApplet.minAutoFillLength;
                        } else if (isNeutral) {
                            neutralAppletsNo = neutralAppletsNo + 1;
                            neutralApplets.push(curApplet);
                        }
                    }
                }

                if (mostDemandingApplet) {
                    //! the most demanding applet gains all the remaining space

                    if (inMaxAutoFillCalculations) {
                        mostDemandingApplet.maxAutoFillLength = mostDemandingApplet.maxAutoFillLength + sizePerApplet;
                    } else {
                        mostDemandingApplet.minAutoFillLength = mostDemandingApplet.minAutoFillLength + sizePerApplet;
                    }

                    // console.log("s4_1  "+ mostDemandingApplet.applet.pluginName + " assigned: "  + mostDemandingApplet.maxAutoFillLength + "\n");
                } else if (neutralAppletsNo>0) {
                    //! if no demanding applets was found then the available space is splitted equally
                    //! between all neutralApplets
                    var adjustedAppletSize = (sizePerApplet / neutralAppletsNo);
                    for (var j=0; j<neutralApplets.length; ++j) {
                        // console.log("s4_2.0  "+ neutralApplets[j].applet.pluginName + " _ " + neutralApplets[j].maxAutoFillLength + " _ " + adjustedAppletSize);

                        if (inMaxAutoFillCalculations) {
                            neutralApplets[j].maxAutoFillLength = neutralApplets[j].maxAutoFillLength + adjustedAppletSize;
                        } else {
                            neutralApplets[j].minAutoFillLength = neutralApplets[j].minAutoFillLength + adjustedAppletSize;
                        }

                        // console.log("s4_2  "+ neutralApplets[j].applet.pluginName + " assigned: "  + sizePerApplet + "\n");
                    }
                }
            } else {
                for(var i=0; i<layout.children.length; ++i) {
                    var curApplet = layout.children[i];

                    if (curApplet && curApplet.isAutoFillApplet && curApplet.inFillCalculations) {
                        if (inMaxAutoFillCalculations) {
                            curApplet.maxAutoFillLength = sizePerApplet;
                        } else {
                            curApplet.minAutoFillLength = sizePerApplet;
                        }

                        // console.log("s4_3  "+ curApplet.applet.pluginName + " assigned: "  + sizePerApplet + "\n");
                        curApplet.inFillCalculations = false;
                    }
                }
            }
        }
    }

    //! initialize the three layouts and execute the step1/phase1
    //! it is used when the Centered (Main)Layout is used only or when the Main(Layout)
    //! is empty in Justify mode
    function initializationPhase(availableSpace, sizePerApplet, noOfApplets, inMaxAutoFillCalculations){
        if (root.panelAlignment === LatteCore.Types.Justify) {
            initLayoutForFillsCalculations(startLayout.grid);
            initLayoutForFillsCalculations(endLayout.grid);
        }
        initLayoutForFillsCalculations(mainLayout.grid);

        // console.log("s3...");

        //! first pass in order to update sizes for applet that want to fill space
        //! but their maximum metrics are lower than the sizePerApplet
        var res = computeStep1ForLayout(mainLayout.grid, availableSpace, sizePerApplet, noOfApplets, inMaxAutoFillCalculations);
        availableSpace = res[0]; sizePerApplet = res[1]; noOfApplets = res[2];

        // console.log( " i1 : " + availableSpace + " _ " + sizePerApplet + " _ " + noOfApplets );

        if (root.panelAlignment === LatteCore.Types.Justify) {
            res = computeStep1ForLayout(startLayout.grid, availableSpace, sizePerApplet, noOfApplets, inMaxAutoFillCalculations);
            availableSpace = res[0]; sizePerApplet = res[1]; noOfApplets = res[2];
            // console.log( " i2 : " + availableSpace + " _ " + sizePerApplet + " _ " + noOfApplets );

            res = computeStep1ForLayout(endLayout.grid, availableSpace, sizePerApplet, noOfApplets, inMaxAutoFillCalculations);
            availableSpace = res[0]; sizePerApplet = res[1]; noOfApplets = res[2];
            // console.log( " i3 : " + availableSpace + " _ " + sizePerApplet + " _ " + noOfApplets );
        }

        return [availableSpace, sizePerApplet, noOfApplets];
    }

    function updateFillAppletsWithTwoSteps(inMaxAutoFillCalculations) {
        var noA = startLayout.fillApplets + mainLayout.fillApplets + endLayout.fillApplets;
        var max_length = inMaxAutoFillCalculations ? root.maxLength : root.minLength

        // console.log(" S3 _ SIZES ::: " + max_length + " ___ " + inMaxAutoFillCalculations + " __ " + startLayout.sizeWithNoFillApplets + " ___ " + mainLayout.sizeWithNoFillApplets + " ___ " + endLayout.sizeWithNoFillApplets);

        //! compute the two free spaces around the centered layout
        //! they are called start and end accordingly
        var halfMainLayout = mainLayout.sizeWithNoFillApplets / 2;
        var availableSpaceStart = Math.max(0, max_length/2 - startLayout.sizeWithNoFillApplets - halfMainLayout - root.panelEdgeSpacing/2);
        var availableSpaceEnd = Math.max(0, max_length/2 - endLayout.sizeWithNoFillApplets - halfMainLayout - root.panelEdgeSpacing/2);
        var availableSpace;

        if (mainLayout.fillApplets === 0 || (startLayout.shownApplets ===0 && endLayout.shownApplets===0)){
            //! no fill applets in main OR we are in alignment that all applets are in main
            availableSpace = availableSpaceStart + availableSpaceEnd - mainLayout.sizeWithNoFillApplets;
        } else {
            //! use the minimum available space in order to avoid overlaps
            availableSpace = 2 * Math.min(availableSpaceStart, availableSpaceEnd) - mainLayout.sizeWithNoFillApplets;
        }

        var sizePerAppletMain = mainLayout.fillApplets > 0 ? availableSpace / noA : 0 ;

        var noStart = startLayout.fillApplets;
        var noMain = mainLayout.fillApplets;
        var noEnd = endLayout.fillApplets;

        //! initialize the computations
        initLayoutForFillsCalculations(startLayout.grid, inMaxAutoFillCalculations);
        initLayoutForFillsCalculations(mainLayout.grid, inMaxAutoFillCalculations);
        initLayoutForFillsCalculations(endLayout.grid, inMaxAutoFillCalculations);

        var res;

        //! first pass
        if (mainLayout.fillApplets > 0){
            res = computeStep1ForLayout(mainLayout.grid, availableSpace, sizePerAppletMain, noMain, inMaxAutoFillCalculations);
            sizePerAppletMain = res[1]; noMain = res[2];
            var dif = (availableSpace - res[0]) / 2;
            availableSpaceStart = availableSpaceStart - dif;
            availableSpaceEnd = availableSpaceEnd - dif;
        }

        var sizePerAppletStart = startLayout.fillApplets > 0 ? availableSpaceStart / noStart : 0 ;
        var sizePerAppletEnd = endLayout.fillApplets > 0 ? availableSpaceEnd / noEnd : 0 ;

        if (startLayout.fillApplets > 0) {
            res = computeStep1ForLayout(startLayout.grid, availableSpaceStart, sizePerAppletStart, noStart, inMaxAutoFillCalculations);
            availableSpaceStart = res[0]; sizePerAppletStart = res[1]; noStart = res[2];
        }
        if (endLayout.fillApplets > 0) {
            res = computeStep1ForLayout(endLayout.grid, availableSpaceEnd, sizePerAppletEnd, noEnd, inMaxAutoFillCalculations);
            availableSpaceEnd = res[0]; sizePerAppletEnd = res[1]; noEnd = res[2];
        }

        ////
        //! second pass

        // console.log(" S ::: " +startLayout.fillApplets + " _ " + sizePerAppletStart + " _ " + noStart);

        if (mainLayout.fillApplets > 0) {
            computeStep2ForLayout(mainLayout.grid, sizePerAppletMain, noMain, inMaxAutoFillCalculations); //default behavior
        }

        if (startLayout.fillApplets > 0) {
            if (mainLayout.fillApplets > 0) {
                //! adjust final fill applet size in mainlayouts final length
                sizePerAppletStart = ((max_length/2) - (mainLayout.grid.length/2) - startLayout.sizeWithNoFillApplets) / noStart;
            }

            computeStep2ForLayout(startLayout.grid, sizePerAppletStart, noStart, inMaxAutoFillCalculations);
        }

        if (endLayout.fillApplets > 0) {
            if (mainLayout.fillApplets > 0) {
                //! adjust final fill applet size in mainlayouts final length
                sizePerAppletEnd = ((max_length/2) - (mainLayout.grid.length/2) - endLayout.sizeWithNoFillApplets) / noEnd;
            }

            computeStep2ForLayout(endLayout.grid, sizePerAppletEnd, noEnd, inMaxAutoFillCalculations);
        }

    }

    function updateFillAppletsWithOneStep(inMaxAutoFillCalculations) {
        var max_length = inMaxAutoFillCalculations ? root.maxLength : root.minLength
        var noA = startLayout.fillApplets + mainLayout.fillApplets + endLayout.fillApplets;

        // console.log("  S2 _ SIZES ::: " + max_length + " ___ " + inMaxAutoFillCalculations + " __ " + startLayout.sizeWithNoFillApplets + " ___ " + mainLayout.sizeWithNoFillApplets + " ___ " + endLayout.sizeWithNoFillApplets);

        var availableSpace = Math.max(0, max_length - startLayout.sizeWithNoFillApplets - mainLayout.sizeWithNoFillApplets - endLayout.sizeWithNoFillApplets - root.panelEdgeSpacing);
        var sizePerApplet = availableSpace / noA;

        var res = initializationPhase(availableSpace, sizePerApplet, noA, inMaxAutoFillCalculations);
        availableSpace = res[0];  sizePerApplet = res[1]; noA = res[2];

        // console.log("s4...");

        //! after step1 there is a chance that all applets were assigned a valid space
        //! but at the same time some space remained free. In such case we make sure
        //! that remained space will be assigned to the most demanding applet.
        //! This is achieved by <layout>No values. For step2 passing value!=0
        //! means default step2 behavior BUT value=0 means that remained space
        //! must be also assigned at the end.
        var remainedSpace = (noA === 0 && sizePerApplet > 0) ? true : false

        var startNo = -1;
        var mainNo = -1;
        var endNo = -1;

        if (remainedSpace) {
            if (startLayout.fillApplets > 0) {
                startNo = 0;
            } else if (lendLayout.fillApplets > 0) {
                endNo = 0;
            } else if (mainLayout.fillApplets > 0) {
                mainNo = 0;
            }
        }

        //! second pass in order to update sizes for applet that want to fill space
        //! these applets get the direct division of the available free space that
        //! remained from step1 OR the the free available space that no applet requested yet

        computeStep2ForLayout(startLayout.grid, sizePerApplet, startNo, inMaxAutoFillCalculations); //default behavior
        computeStep2ForLayout(mainLayout.grid, sizePerApplet, mainNo, inMaxAutoFillCalculations); //default behavior
        computeStep2ForLayout(endLayout.grid, sizePerApplet, endNo, inMaxAutoFillCalculations); //default behavior

        //console.log("s5...");
    }


    function _updateSizeForAppletsInFill() {
        if (inNormalFillCalculationsState) {
            // console.log("-------------");
            // console.log("s1...");
            var noA = startLayout.fillApplets + mainLayout.fillApplets + endLayout.fillApplets;

            if (noA === 0) {
                return;
            }

            var use_maximum_length = true;

            if (mainLayout.shownApplets === 0 || root.panelAlignment !== LatteCore.Types.Justify) {
                updateFillAppletsWithOneStep(use_maximum_length);
                updateFillAppletsWithOneStep(!use_maximum_length);
            } else {
                //! Justify mode in all remaining cases
                updateFillAppletsWithTwoSteps(use_maximum_length);
                updateFillAppletsWithTwoSteps(!use_maximum_length);
            }
        }
    }
}
