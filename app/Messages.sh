#! /usr/bin/env bash
$XGETTEXT `find . ../shell -name \*.js -o -name \*.qml -o -name \*.cpp` -o $podir/latte-dock.pot
