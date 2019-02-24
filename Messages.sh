#! /usr/bin/env bash
$EXTRACTRC `find app -name \*.rc -o -name \*.ui` >> rc.cpp

$XGETTEXT `find app shell rc.cpp -name \*.js -o -name \*.qml -o -name \*.cpp` -o $podir/latte-dock.pot
