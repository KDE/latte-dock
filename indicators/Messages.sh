#! /usr/bin/env bash

$XGETTEXT `find default -name \*.js -o -name \*.qml -o -name \*.cpp` -o $podir/latte_indicator_org.kde.latte.default.pot
$XGETTEXT `find org.kde.latte.plasma -name \*.js -o -name \*.qml -o -name \*.cpp` -o $podir/latte_indicator_org.kde.latte.plasma.pot
$XGETTEXT `find org.kde.latte.plasmatabstyle -name \*.js -o -name \*.qml -o -name \*.cpp` -o $podir/latte_indicator_org.kde.latte.plasmatabstyle.pot 
