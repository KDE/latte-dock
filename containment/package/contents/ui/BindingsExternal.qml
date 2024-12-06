/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore
import org.kde.latte.private.containment 0.1 as LatteContainment

Item {
    property bool updateIsEnabled: autosize.inCalculatedIconSize
                                   && !visibilityManager.inSlidingIn
                                   && !visibilityManager.inSlidingOut
                                   && !visibilityManager.inRelocationHiding

    //! Latte::View Main Bindings 
    Binding{
        target: latteView
        property:"maxThickness"
        //! prevents updating window geometry during closing window in wayland and such fixes a crash
        when: latteView && !visibilityManager.inRelocationHiding && !visibilityManager.inClientSideScreenEdgeSliding //&& !inStartup
        value: root.behaveAsPlasmaPanel ? visibilityManager.thicknessAsPanel : metrics.maxThicknessForView
    }

    Binding{
        target: latteView
        property:"normalThickness"
        when: latteView && updateIsEnabled
        value: root.behaveAsPlasmaPanel ? visibilityManager.thicknessAsPanel : metrics.mask.screenEdge + metrics.mask.thickness.maxNormalForItemsWithoutScreenEdge
    }

    Binding{
        target: latteView
        property:"maxNormalThickness"
        when: latteView && updateIsEnabled
        value: metrics.mask.thickness.maxNormal
    }

    Binding {
        target: latteView
        property: "headThicknessGap"
        when: latteView && updateIsEnabled && !visibilityManager.inClientSideScreenEdgeSliding
        value: {
            if (root.behaveAsPlasmaPanel || root.viewType === LatteCore.Types.PanelView || (latteView && latteView.byPassWM)) {
                return 0;
            }

            return metrics.maxThicknessForView - metrics.mask.thickness.maxNormalForItems;
        }
    }

    Binding{
        target: latteView
        property: "type"
        when: latteView
        value: root.viewType
    }

    Binding{
        target: latteView
        property: "behaveAsPlasmaPanel"
        when: latteView
        value: root.behaveAsPlasmaPanel
    }

    Binding{
        target: latteView
        property: "fontPixelSize"
        when: theme
        value: theme.defaultFont.pixelSize
    }

    Binding{
        target: latteView
        property: "maxLength"
        when: latteView
        value: root.maxLengthPerCentage/100
    }

    Binding{
        target: latteView
        property: "offset"
        when: latteView
        value: plasmoid.configuration.offset/100
    }

    Binding{
        target: latteView
        property: "screenEdgeMargin"
        when: latteView
        value: Math.max(0, plasmoid.configuration.screenEdgeMargin)
    }

    Binding{
        target: latteView
        property: "screenEdgeMarginEnabled"
        when: latteView
        value: root.screenEdgeMarginEnabled && !root.hideThickScreenGap
    }

    Binding{
        target: latteView
        property: "alignment"
        when: latteView
        value: myView.alignment
    }

    Binding{
        target: latteView
        property: "isTouchingTopViewAndIsBusy"
        when: root.viewIsAvailable
        value: {
            if (!root.viewIsAvailable) {
                return false;
            }

            var isTouchingTopScreenEdge = (latteView.y === latteView.screenGeometry.y);
            var isStickedOnTopBorder = (plasmoid.configuration.alignment === LatteCore.Types.Justify && plasmoid.configuration.maxLength===100)
                    || (plasmoid.configuration.alignment === LatteCore.Types.Top && plasmoid.configuration.offset===0);

            return root.isVertical && !latteView.visibility.isHidden && !isTouchingTopScreenEdge && isStickedOnTopBorder && background.isShown;
        }
    }

    Binding{
        target: latteView
        property: "isTouchingBottomViewAndIsBusy"
        when: latteView
        value: {
            if (!root.viewIsAvailable) {
                return false;
            }

            var latteBottom = latteView.y + latteView.height;
            var screenBottom = latteView.screenGeometry.y + latteView.screenGeometry.height;
            var isTouchingBottomScreenEdge = (latteBottom === screenBottom);

            var isStickedOnBottomBorder = (plasmoid.configuration.alignment === LatteCore.Types.Justify && plasmoid.configuration.maxLength===100)
                    || (plasmoid.configuration.alignment === LatteCore.Types.Bottom && plasmoid.configuration.offset===0);

            return root.isVertical && !latteView.visibility.isHidden && !isTouchingBottomScreenEdge && isStickedOnBottomBorder && background.isShown;
        }
    }

    Binding{
        target: latteView
        property: "colorizer"
        when: latteView
        value: colorizerManager
    }

    Binding{
        target: latteView
        property: "metrics"
        when: latteView
        value: metrics
    }

    //! View::Effects bindings
    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property: "backgroundAllCorners"
        when: latteView && latteView.effects
        value: plasmoid.configuration.backgroundAllCorners
               && (!root.screenEdgeMarginEnabled /*no-floating*/
                   || (root.screenEdgeMarginEnabled /*floating with justify alignment and 100% maxlength*/
                       && plasmoid.configuration.maxLength===100
                       && myView.alignment===LatteCore.Types.Justify
                       && !root.hideLengthScreenGaps))
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property: "backgroundRadius"
        when: latteView && latteView.effects
        value: background.customRadius
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property: "backgroundRadiusEnabled"
        when: latteView && latteView.effects
        value: background.customRadiusIsEnabled
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property: "backgroundOpacity"
        when: latteView && latteView.effects
        value: plasmoid.configuration.panelTransparency===-1 /*Default option*/ ? -1 : background.currentOpacity
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property: "drawEffects"
        when: latteView && latteView.effects && !root.inStartup
        value: LatteCore.WindowSystem.compositingActive
               && (((root.blurEnabled && root.useThemePanel) || (root.blurEnabled && root.forceSolidPanel))
                   && (!root.inStartup || visibilityManager.inRelocationHiding))
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property: "drawShadows"
        when: latteView && latteView.effects
        value: root.drawShadowsExternal && (!root.inStartup || visibilityManager.inRelocationHiding) && !(latteView && latteView.visibility.isHidden)
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property:"editShadow"
        when: latteView && latteView.effects
        value: root.editShadow
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property:"innerShadow"
        when: latteView && latteView.effects
        value: background.shadows.headThickness
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property: "panelBackgroundSvg"
        when: latteView && latteView.effects
        value: background.panelBackgroundSvg
    }

    Binding{
        target: latteView && latteView.effects ? latteView.effects : null
        property:"appletsLayoutGeometry"
        when: latteView && latteView.effects && visibilityManager.inNormalState
        value: {
            if (root.behaveAsPlasmaPanel
                    || !LatteCore.WindowSystem.compositingActive
                    || (!parabolic.isEnabled && root.userShowPanelBackground && plasmoid.configuration.panelSize===100)) {
                var paddingtail = background.tailRoundness + background.tailRoundnessMargin;
                var paddinghead = background.headRoundness + background.headRoundnessMargin;

                if (root.isHorizontal) {
                    return Qt.rect(latteView.localGeometry.x + paddingtail,
                                   latteView.localGeometry.y,
                                   latteView.localGeometry.width - paddingtail - paddinghead,
                                   latteView.localGeometry.height);
                } else {
                    return Qt.rect(latteView.localGeometry.x,
                                   latteView.localGeometry.y + paddingtail,
                                   latteView.localGeometry.width,
                                   latteView.localGeometry.height - paddingtail - paddinghead);
                }
            }

            return Qt.rect(-1, -1, 0, 0);
        }
    }

    //! View::Positioner bindings
    Binding{
        target: latteView && latteView.positioner ? latteView.positioner : null
        property: "isStickedOnTopEdge"
        when: latteView && latteView.positioner
        value: plasmoid.configuration.isStickedOnTopEdge
    }

    Binding{
        target: latteView && latteView.positioner ? latteView.positioner : null
        property: "isStickedOnBottomEdge"
        when: latteView && latteView.positioner
        value: plasmoid.configuration.isStickedOnBottomEdge
    }

    //! View::VisibilityManager
    Binding{
        target: latteView && latteView.visibility ? latteView.visibility : null
        property: "isShownFully"
        when: latteView && latteView.visibility
        value: myView.isShownFully
    }

    Binding{
        target: latteView && latteView.visibility ? latteView.visibility : null
        property: "strutsThickness"
        when: latteView && latteView.visibility
        value: {
            var isCapableToHideScreenGap = root.screenEdgeMarginEnabled && plasmoid.configuration.hideFloatingGapForMaximized
            var mirrorGapFactor = root.mirrorScreenGap ? 2 : 1;

            //! Hide Thickness Screen Gap scenario provides two different struts thicknesses.
            //! [1] The first struts thickness is when there is no maximized window and is such case
            //!     the view is behaving as in normal AlwaysVisible visibility mode. This is very useful
            //!     when users tile windows. [bug #432122]
            //! [2] The second struts thickness is when there is a maximized window present and in such case
            //!     the view is hiding all of its screen edges. It is used mostly when the view is wanted
            //!     to act as a window titlebar.
            var thicknessForIsCapableToHideScreenGap = (root.hideThickScreenGap ? 0 : mirrorGapFactor * metrics.mask.screenEdge);

            if (root.behaveAsPlasmaPanel) {
                return isCapableToHideScreenGap ?
                            (visibilityManager.thicknessAsPanel + thicknessForIsCapableToHideScreenGap) :
                            (mirrorGapFactor*metrics.mask.screenEdge) + visibilityManager.thicknessAsPanel;
            }

            var edgeThickness = isCapableToHideScreenGap ? thicknessForIsCapableToHideScreenGap : metrics.mask.screenEdge * mirrorGapFactor;
            return edgeThickness + metrics.mask.thickness.maxNormalForItemsWithoutScreenEdge;
        }
    }

    Binding {
        target: latteView && latteView.visibility ? latteView.visibility : null
        property: "isFloatingGapWindowEnabled"
        when: latteView && latteView.visibility
        value: root.hasFloatingGapInputEventsDisabled
               && (latteView.visibility.mode === LatteCore.Types.AutoHide
                   || latteView.visibility.mode === LatteCore.Types.DodgeActive
                   || latteView.visibility.mode === LatteCore.Types.DodgeAllWindows
                   || latteView.visibility.mode === LatteCore.Types.DodgeMaximized
                   || latteView.visibility.mode === LatteCore.Types.SidebarAutoHide)
    }

    //! View::WindowsTracker bindings
    Binding{
        target: latteView && latteView.windowsTracker ? latteView.windowsTracker : null
        property: "enabled"
        //! During startup phase windows tracking is not enabled and does not
        //! influence startup sequence at all. At the same time no windows tracking
        //! takes place during startup and as such startup time is reduced
        when: latteView && latteView.windowsTracker && latteView.visibility && !root.inStartup
        value: (latteView && latteView.visibility
                && !(latteView.visibility.mode === LatteCore.Types.AlwaysVisible /* Visibility */
                     || latteView.visibility.mode === LatteCore.Types.WindowsGoBelow
                     || latteView.visibility.mode === LatteCore.Types.AutoHide))
               || indexer.clientsTrackingWindowsCount  > 0                   /*Applets Need Windows Tracking */
               || root.dragActiveWindowEnabled                               /*Dragging Active Window(Empty Areas)*/
               || ((root.backgroundOnlyOnMaximized                           /*Dynamic Background */
                    || plasmoid.configuration.solidBackgroundForMaximized
                    || root.disablePanelShadowMaximized
                    || root.windowColors !== LatteContainment.Types.NoneWindowColors))
               || (root.screenEdgeMarginsEnabled                             /*Dynamic Screen Edge Margin*/
                   && plasmoid.configuration.hideFloatingGapForMaximized)
    }

    //! View::ExtendedInterface bindings
    Binding{
        target: latteView && latteView.extendedInterface ? latteView.extendedInterface : null
        property: "plasmoid"
        when: latteView && latteView.extendedInterface
        value: plasmoid
    }

    Binding{
        target: latteView && latteView.extendedInterface ? latteView.extendedInterface : null
        property: "layoutManager"
        when: latteView && latteView.extendedInterface
        value: fastLayoutManager
    }
}
