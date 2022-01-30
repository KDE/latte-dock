/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.latte.core 0.2 as LatteCore

Window{
    width: mainGrid.width + 10
    height: Math.min(mainGrid.height+10, Screen.height - metrics.mask.thickness.maxNormalForItems)
    visible: true
    title: "#debugwindow#"

    property string space:" :   "

    PlasmaExtras.ScrollArea {
        id: scrollArea

        anchors.fill: parent
        verticalScrollBarPolicy: Qt.ScrollBarAsNeeded
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

        flickableItem.flickableDirection: Flickable.VerticalFlick

        Grid{
            id:mainGrid
            columns: 2

            Text{
                text: "Name"+space
            }

            Text{
                text: latteView && latteView.name.length > 0 ? latteView.name : "___"
            }

            Text{
                text: "Screen id"+space
            }

            Text{
                text: latteView && latteView.positioner ? latteView.positioner.currentScreenName : "___"
            }

            Text{
                text: "Screen Geometry"+space
            }

            Text{
                text: {
                    if (latteView && latteView.screenGeometry){
                        return latteView.screenGeometry.x+","+latteView.screenGeometry.y+ " "+latteView.screenGeometry.width+"x"+latteView.screenGeometry.height;
                    } else {
                        return "_,_ _x_";
                    }
                }
            }

            Text{
                text: "Window Geometry"+space
            }

            Text{
                text: {
                    if (latteView) {
                        return  latteView.x + "," + latteView.y + " "+latteView.width+ "x"+latteView.height;
                    } else {
                        return "_,_ _x_";
                    }
                }
            }

            Text{
                text: "On Primary"+space
            }

            Text{
                text: {
                    if (latteView && latteView.onPrimary)
                        return "Yes";
                    else
                        return "No";
                }
            }

            Text{
                text: "   -----------   "
            }

            Text{
                text: " -----------   "
            }

            Text{
                text: "Contents Width"+space
            }

            Text{
                text: layoutsContainer.contentsWidth
            }

            Text{
                text: "Contents Height"+space
            }

            Text{
                text: layoutsContainer.contentsHeight
            }

            Text{
                text: "Max Length"+space
            }

            Text{
                text: root.maxLengthPerCentage +"%"
            }

            Text{
                text: "Max Length (pixels)"+space
            }

            Text{
                text: root.maxLength
            }

            Text{
                text: "Min Length"+space
            }

            Text{
                text: root.minLengthPerCentage +"%"
            }

            Text{
                text: "Min Length (pixels)"+space
            }

            Text{
                text: root.minLength
            }

            Text{
                text: "Offset (pixels)"+space
            }

            Text{
                text: root.offset
            }

            Text{
                text: "Mask"+space
            }

            Text{
                text: {
                    if (latteView && latteView.effects && latteView.effects.mask) {
                        return latteView.effects.mask.x +", "+ latteView.effects.mask.y+"  "+latteView.effects.mask.width+"x"+latteView.effects.mask.height;
                    } else {
                        return "_,_ _x_";
                    }
                }
            }

            Text{
                text: "Input "+space
            }

            Text{
                text: {
                    if (latteView && latteView.effects && latteView.effects.inputMask) {
                        return latteView.effects.inputMask.x +", "+ latteView.effects.inputMask.y+"  "+latteView.effects.inputMask.width+"x"+latteView.effects.inputMask.height;
                    } else {
                        return "_,_ _x_";
                    }
                }
            }

            Text{
                text: "Local Geometry"+space
            }

            Text{
                text: {
                    if (latteView && latteView.localGeometry) {
                        return latteView.localGeometry.x + ", " + latteView.localGeometry.y + "  " + latteView.localGeometry.width + "x" + latteView.localGeometry.height;
                    } else {
                        return "_,_ _x_";
                    }
                }
            }

            Text{
                text: "Absolute Geometry"+space
            }

            Text{
                text: {
                    if (latteView && latteView.absoluteGeometry) {
                        return latteView.absoluteGeometry.x + ", " + latteView.absoluteGeometry.y + "  " + latteView.absoluteGeometry.width + "x" + latteView.absoluteGeometry.height;
                    } else {
                        return "_,_ _x_";
                    }
                }
            }


            Text{
                text: "Draw Effects"+space
            }

            Text{
                text: {
                    if (latteView && latteView.effects && latteView.effects.drawEffects)
                        return "Yes";
                    else
                        return "No";
                }
            }

            Text{
                text: "Effects Area"+space
            }

            Text{
                text: {
                    if (latteView && latteView.effects && latteView.effects.rect) {
                        return latteView.effects.rect.x + ", " + latteView.effects.rect.y + "  " +latteView.effects.rect.width + "x" + latteView.effects.rect.height;
                    } else {
                        return "_,_ _x_";
                    }
                }
            }

            Text{
                text: "   -----------   "
            }

            Text{
                text: " -----------   "
            }

            Text{
                text: "Is Hidden (flag)"+space
            }

            Text{
                text: {
                    if (latteView && latteView.visibility && latteView.visibility.isHidden)
                        return "Yes";
                    else
                        return "No";
                }
            }

            Text{
                text: "Contains Mouse (flag)"+space
            }

            Text{
                text: {
                    if (latteView && latteView.visibility && latteView.visibility.containsMouse)
                        return "Yes";
                    else
                        return "No";
                }
            }

            Text{
                text: "Edit Mode"+space
            }

            Text{
                text: {
                    if (root.editMode)
                        return "Yes";
                    else
                        return "No";
                }
            }

            Text{
                text: "   -----------   "
            }

            Text{
                text: " -----------   "
            }

            Text{
                text: "Location"+space
            }

            Text{
                text: {
                    switch(plasmoid.location){
                    case PlasmaCore.Types.LeftEdge:
                        return "Left Edge";
                        break;
                    case PlasmaCore.Types.RightEdge:
                        return "Right Edge";
                        break;
                    case PlasmaCore.Types.TopEdge:
                        return "Top Edge";
                        break;
                    case PlasmaCore.Types.BottomEdge:
                        return "Bottom Edge";
                        break;
                    }

                    return " <unknown> : " + plasmoid.location;
                }
            }

            Text{
                text: "Alignment"+space
            }

            Text{
                text: {
                    switch(plasmoid.configuration.alignment){
                    case LatteCore.Types.Left:
                        return "Left";
                        break;
                    case LatteCore.Types.Right:
                        return "Right";
                        break;
                    case LatteCore.Types.Center:
                        return "Center";
                        break;
                    case LatteCore.Types.Top:
                        return "Top";
                        break;
                    case LatteCore.Types.Bottom:
                        return "Bottom";
                        break;
                    case LatteCore.Types.Justify:
                        return "Justify";
                        break;
                    }

                    return "<unknown> : " + plasmoid.configuration.alignment;
                }
            }

            Text{
                text: "Visibility"+space
            }

            Text{
                text: {
                    if (!latteView || !latteView.visibility)
                        return "";

                    switch(latteView.visibility.mode){
                    case LatteCore.Types.AlwaysVisible:
                        return "Always Visible";
                        break;
                    case LatteCore.Types.AutoHide:
                        return "Auto Hide";
                        break;
                    case LatteCore.Types.DodgeActive:
                        return "Dodge Active";
                        break;
                    case LatteCore.Types.DodgeMaximized:
                        return "Dodge Maximized";
                        break;
                    case LatteCore.Types.DodgeAllWindows:
                        return "Dodge All Windows";
                        break;
                    case LatteCore.Types.DodgeAllWindows:
                        return "Dodge All Windows";
                        break;
                    case LatteCore.Types.WindowsGoBelow:
                        return "Windows Go Below";
                        break;
                    case LatteCore.Types.WindowsCanCover:
                        return "Windows Can Cover";
                        break;
                    case LatteCore.Types.WindowsAlwaysCover:
                        return "Windows Always Cover";
                        break;
                    case LatteCore.Types.SidebarOnDemand:
                        return "OnDemand Sidebar";
                        break;
                    case LatteCore.Types.SidebarAutoHide:
                        return "AutoHide Sidebar";
                    break;
                    case LatteCore.Types.NormalWindow:
                        return "Normal Window";
                        break;
                    case LatteCore.Types.None:
                        return "None";
                        break;
                    }

                    return "<unknown> : " + latteView.visibility.mode;
                }
            }

            Text{
                text: "Zoom Factor"+space
            }

            Text{
                text: parabolic.factor.zoom
            }

            Text{
                text: "   -----------   "
            }

            Text{
                text: " -----------   "
            }

            Text{
                text: "Icon Size (current)"+space
            }

            Text{
                text: metrics.iconSize
            }

            Text{
                text: "Icon Size (user)"+space
            }

            Text{
                text: plasmoid.configuration.iconSize
            }

            Text{
                text: "Icon Size (portion)"+space
            }

            Text{
                text: metrics.portionIconSize
            }

            Text{
                text: "Icon Size (auto decrease), Enabled"+space
            }

            Text{
                text: {
                    if (autosize.isActive)
                        return "Yes";
                    else
                        return "No";
                }
            }

            Text{
                text: "Icon Size (auto decrease)"+space
            }

            Text{
                text: autosize.iconSize
            }

            Text{
                text: "Length Padding (pixels)"+space
            }

            Text{
                text: metrics.padding.length
            }

            Text{
                text: "Length Margin (pixels)"+space
            }

            Text{
                text: metrics.margin.length
            }

            Text{
                text: "Head Thickness Margin"+space
            }

            Text{
                text: metrics.margin.headThickness
            }

            Text{
                text: "Tail Thickness Margin"+space
            }

            Text{
                text: metrics.margin.tailThickness
            }

            Text{
                text: "   -----------   "
            }

            Text{
                text: " -----------   "
            }

            Text{
                text: "Show Panel Background (user)"+space
            }

            Text{
                text: {
                    if (plasmoid.configuration.useThemePanel)
                        return "Yes";
                    else
                        return "No";
                }
            }

            Text{
                text: "Force Transparent Panel (auto)"+space
            }

            Text{
                text: {
                    if (root.forceTransparentPanel)
                        return "Yes";
                    else
                        return "No";
                }
            }

            Text{
                text: "Background Length"+space
            }

            Text{
                text: background.totals.visualLength
            }

            Text{
                text: "Background Thickness(user)"+space
            }

            Text{
                text: plasmoid.configuration.panelSize + "%"
            }

            Text{
                text: "Background Thickness(auto)"+space
            }

            Text{
                text: background.totals.visualThickness
            }

            Text{
                text: "Background Opacity"+space
            }

            Text{
                text: (root.myView.backgroundOpacity * 100) + "%"
            }

            Text{
                text: "Background Shadows Active"+space
            }

            Text{
                text: {
                    if (root.panelShadowsActive)
                        return "Yes";
                    else
                        return "No";
                }
            }

            Text{
                text: "Background Shadow"+space
            }

            Text{
                text: background.shadows.headThickness
            }

            Text{
                text: "Background Head Thickness Padding"+space
            }

            Text{
                text: background.paddings.headThickness
            }

            Text{
                text: "   -----------   "
            }

            Text{
                text: " -----------   "
            }

            Text{
                text: "Mask - Normal Thickness"+space
            }

            Text{
                text: metrics.mask.thickness.normal
            }

            Text{
                text: "Thickness Uses Panel Size"+space
            }

            Text{
                text: background.isGreaterThanItemThickness
            }

            Text{
                text: "Behave As Plasma Panel"+space
            }

            Text{
                text: {
                    if (root.behaveAsPlasmaPanel)
                        return "Yes";
                    else
                        return "No";
                }
            }

            Text{
                text: "Draw Shadows (external)"+space
            }

            Text{
                text: {
                    if (root.drawShadowsExternal)
                        return "Yes";
                    else
                        return "No";
                }
            }

            Text{
                text: "   -----------   "
            }

            Text{
                text: " -----------   "
            }

            Text{
                text: "Applet Hovered"+space
            }

            Text{
                text: "--"//layoutsContainer.hoveredIndex
            }

            Text{
                text: "In Normal State"+space
            }

            Text{
                text: visibilityManager.inNormalState
            }

            Text{
                text: "Animations Both Axis"+space
            }

            Text{
                text: animations.needBothAxis.count
            }

            Text{
                text: "Animations Only Length"+space
            }

            Text{
                text: animations.needLength.count
            }

            Text{
                text: "Animations Need Thickness"+space
            }

            Text{
                text: animations.needThickness.count
            }

            Text{
                text: "   -----------   "
            }

            Text{
                text: " -----------   "
            }

            Text{
                text: "Start Layout Shown Applets"+space
            }

            Text{
                text: layouter.startLayout.shownApplets
            }

            Text{
                text: "Start Layout Applets (with fill)"+space
            }

            Text{
                text: layouter.startLayout.fillApplets
            }

            Text{
                text: "Start Layout Size (no fill applets)"+space
            }

            Text{
                text: layouter.startLayout.sizeWithNoFillApplets+" px."
            }

            Text{
                text: "   -----------   "
            }

            Text{
                text: " -----------   "
            }

            Text{
                text: "Main Layout Shown Applets"+space
            }

            Text{
                text: layouter.mainLayout.shownApplets
            }

            Text{
                text: "Main Layout Applets (with fill)"+space
            }

            Text{
                text: layouter.mainLayout.fillApplets
            }

            Text{
                text: "Main Layout Size (no fill applets)"+space
            }

            Text{
                text: layouter.mainLayout.sizeWithNoFillApplets+" px."
            }

            Text{
                text: "   -----------   "
            }

            Text{
                text: " -----------   "
            }

            Text{
                text: "End Layout Shown Applets"+space
            }

            Text{
                text: layouter.endLayout.shownApplets
            }

            Text{
                text: "End Layout Applets (with fill)"+space
            }

            Text{
                text: layouter.endLayout.fillApplets
            }

            Text{
                text: "End Layout Size (no fill applets)"+space
            }

            Text{
                text: layouter.endLayout.sizeWithNoFillApplets+" px."
            }

            Text{
                text: "   -----------   "
            }

            Text{
                text: " -----------   "
            }

            Text{
                text: "Animations ZoomFactor" + space
            }

            Text{
                text: animations.requirements.zoomFactor
            }

            Text{
                text: "   -----------   "
            }

            Text{
                text: " -----------   "
            }

            Text{
                text: "Applets need Windows Tracking"+space
            }

            Text{
                text: indexer.clientsTrackingWindowsCount
            }

            Text{
                text: "Last Active Window Current Screen (id)"+space
            }

            Text{
                text: latteView && latteView.windowsTracker && latteView.windowsTracker.currentScreen.lastActiveWindow && latteView.windowsTracker.currentScreen.lastActiveWindow.isValid ?
                          latteView.windowsTracker.currentScreen.lastActiveWindow.winId : "--"
            }

            Text{
                text: "Last Active Window Current Screen (title)"+space
            }

            Text{
                text: latteView && latteView.windowsTracker && latteView.windowsTracker.currentScreen.lastActiveWindow && latteView.windowsTracker.currentScreen.lastActiveWindow.isValid ?
                          latteView.windowsTracker.currentScreen.lastActiveWindow.display : "--"
                elide: Text.ElideRight
            }

            Text{
                text: "Last Active Window All Screens (id)"+space
            }

            Text{
                text: latteView && latteView.windowsTracker && latteView.windowsTracker.allScreens.lastActiveWindow && latteView.windowsTracker.allScreens.lastActiveWindow.isValid ?
                          latteView.windowsTracker.allScreens.lastActiveWindow.winId : "--"
            }

            Text{
                text: "Last Active Window All Screens (title)"+space
            }

            Text{
                text: latteView && latteView.windowsTracker && latteView.windowsTracker.allScreens.lastActiveWindow && latteView.windowsTracker.allScreens.lastActiveWindow.isValid ?
                          latteView.windowsTracker.allScreens.lastActiveWindow.display : "--"
                elide: Text.ElideRight
            }
        }

    }
}
