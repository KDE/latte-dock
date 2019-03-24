#! /usr/bin/env bash

$XGETTEXT `find default -name \*.js -o -name \*.qml -o -name \*.cpp` -o $podir/latte_indicator_org.kde.latte.indicator.default.pot
$XGETTEXT `find org.kde.latte.indicator.plasma -name \*.js -o -name \*.qml -o -name \*.cpp` -o $podir/latte_indicator_org.kde.latte.indicator.plasma.pot 
