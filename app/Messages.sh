#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc -o -name \*.ui` >> rc.cpp

$XGETTEXT `find . ../shell -name \*.js -o -name \*.qml -o -name \*.cpp -o -name ./dock\/\*.cpp` -o $podir/latte-dock.pot
