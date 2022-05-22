/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import org.kde.plasma.plasmoid 2.0

import org.kde.latte.core 0.2 as LatteCore

import "./privates" as Ability

Ability.ThinTooltipPrivate {
    isEnabled: plasmoid.configuration.titleTooltips
    showIsBlocked: !myView.isShownFully || showIsBlockedFromApplet || layouts.contextMenuIsShown
}
