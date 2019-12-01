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
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.latte 0.2 as Latte

Window{
    width: mainGrid.width + 10
    height: Math.min(mainGrid.height+10, Screen.height - visibilityManager.thicknessNormalOriginal)
    visible: true

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
                text: "Max Length (user)"+space
            }

            Text{
                text: plasmoid.configuration.maxLength +"%"
            }

            Text{
                text: "Max Length (pixels)"+space
            }

            Text{
                text: root.maxLength
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
                text: "Actions Block Hiding "+space
            }

            Text{
                text: root.actionsBlockHiding
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
                    switch(plasmoid.configuration.panelPosition){
                    case Latte.Types.Left:
                        return "Left";
                        break;
                    case Latte.Types.Right:
                        return "Right";
                        break;
                    case Latte.Types.Center:
                        return "Center";
                        break;
                    case Latte.Types.Top:
                        return "Top";
                        break;
                    case Latte.Types.Bottom:
                        return "Bottom";
                        break;
                    case Latte.Types.Justify:
                        return "Justify";
                        break;
                    }

                    return "<unknown> : " + plasmoid.configuration.panelPosition;
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
                    case Latte.Types.AlwaysVisible:
                        return "Always Visible";
                        break;
                    case Latte.Types.AutoHide:
                        return "Auto Hide";
                        break;
                    case Latte.Types.DodgeActive:
                        return "Dodge Active";
                        break;
                    case Latte.Types.DodgeMaximized:
                        return "Dodge Maximized";
                        break;
                    case Latte.Types.DodgeAllWindows:
                        return "Dodge All Windows";
                        break;
                    case Latte.Types.None:
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
                text: root.zoomFactor
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
                text: root.iconSize
            }

            Text{
                text: "Icon Size (user)"+space
            }

            Text{
                text: plasmoid.configuration.iconSize
            }

            Text{
                text: "Icon Size (proportion)"+space
            }

            Text{
                text: root.proportionIconSize
            }

            Text{
                text: "Icon Size (auto decrease), Enabled"+space
            }

            Text{
                text: root.autoDecreaseIconSize
            }

            Text{
                text: "Icon Size (auto decrease)"+space
            }

            Text{
                text: automaticItemSizer.automaticIconSizeBasedSize
            }

            Text{
                text: "Length Internal Margin (pixels)"+space
            }

            Text{
                text: root.lengthIntMargin
            }

            Text{
                text: "Length External Margin (pixels)"+space
            }

            Text{
                text: root.lengthExtMargin
            }

            Text{
                text: "Thickness Margin"+space
            }

            Text{
                text: root.thickMargin
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
                text: "Panel Background Length"+space
            }

            Text{
                text: root.realPanelLength
            }

            Text{
                text: "Panel Background Thickness(user)"+space
            }

            Text{
                text: plasmoid.configuration.panelSize + "%"
            }

            Text{
                text: "Panel Background Thickness(automatic)"+space
            }

            Text{
                text: root.realPanelSize
            }

            Text{
                text: "Panel Transparency"+space
            }

            Text{
                text: root.panelTransparency + "%"
            }

            Text{
                text: "Panel Shadows Active"+space
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
                text: "Panel Background Shadow"+space
            }

            Text{
                text: root.panelShadow
            }

            Text{
                text: "Panel Thickness Margin High"+space
            }

            Text{
                text: root.panelThickMarginHigh
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
                text: visibilityManager.thicknessNormal
            }

            Text{
                text: "Thickness Uses Panel Size"+space
            }

            Text{
                text: visibilityManager.panelIsBiggerFromIconSize
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
                text: layoutsContainer.hoveredIndex
            }

            Text{
                text: "Task Hovered"+space
            }

            Text{
                text: root.latteAppletHoveredIndex
            }

            Text{
                text: "In Normal State"+space
            }

            Text{
                text: visibilityManager.normalState
            }

            Text{
                text: "Animations Both Axis"+space
            }

            Text{
                text: root.animationsNeedBothAxis
            }

            Text{
                text: "Animations Only Length"+space
            }

            Text{
                text: root.animationsNeedLength
            }

            Text{
                text: "Animations Need Thickness"+space
            }

            Text{
                text: root.animationsNeedThickness
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
                text: layoutsContainer.startLayout.shownApplets
            }

            Text{
                text: "Start Layout Applets (with fill)"+space
            }

            Text{
                text: layoutsContainer.startLayout.fillApplets
            }

            Text{
                text: "Start Layout Size (no fill applets)"+space
            }

            Text{
                text: layoutsContainer.startLayout.sizeWithNoFillApplets+" px."
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
                text: layoutsContainer.mainLayout.shownApplets
            }

            Text{
                text: "Main Layout Applets (with fill)"+space
            }

            Text{
                text: layoutsContainer.mainLayout.fillApplets
            }

            Text{
                text: "Main Layout Size (no fill applets)"+space
            }

            Text{
                text: layoutsContainer.mainLayout.sizeWithNoFillApplets+" px."
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
                text: layoutsContainer.endLayout.shownApplets
            }

            Text{
                text: "End Layout Applets (with fill)"+space
            }

            Text{
                text: layoutsContainer.endLayout.fillApplets
            }

            Text{
                text: "End Layout Size (no fill applets)"+space
            }

            Text{
                text: layoutsContainer.endLayout.sizeWithNoFillApplets+" px."
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
                text: root.appletsNeedWindowsTracking
            }

            Text{
                text: "Last Active Window Current Screen (id)"+space
            }

            Text{
                text: latteView && latteView.windowsTracker && latteView.windowsTracker.currentScreen.lastActiveWindow.isValid ?
                          latteView.windowsTracker.currentScreen.lastActiveWindow.winId : "--"
            }

            Text{
                text: "Last Active Window Current Screen (title)"+space
            }

            Text{
                text: latteView && latteView.windowsTracker && latteView.windowsTracker.currentScreen.lastActiveWindow.isValid ?
                          latteView.windowsTracker.currentScreen.lastActiveWindow.display : "--"
                elide: Text.ElideRight
            }

            Text{
                text: "Last Active Window All Screens (id)"+space
            }

            Text{
                text: latteView && latteView.windowsTracker && latteView.windowsTracker.allScreens.lastActiveWindow.isValid ?
                          latteView.windowsTracker.allScreens.lastActiveWindow.winId : "--"
            }

            Text{
                text: "Last Active Window All Screens (title)"+space
            }

            Text{
                text: latteView && latteView.windowsTracker && latteView.windowsTracker.allScreens.lastActiveWindow.isValid ?
                          latteView.windowsTracker.allScreens.lastActiveWindow.display : "--"
                elide: Text.ElideRight
            }
        }

    }
}
