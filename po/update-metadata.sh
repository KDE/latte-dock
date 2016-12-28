#!/bin/sh

PROJECTPATHCONTAINMENT="../../containment" # containment path
PROJECTPATHPLASMOID="../../plasmoid" # plasmoid path
PROJECTPATHSHELL="../../shell" # shell path
PROJECTPATHCORONA="../../corona" # corona path
BUGADDR="https://github.com/psifidotos/latte-dock/" # MSGID-Bugs


cd containment
intltool-merge --quiet --desktop-style . ../../containment.metadata.desktop.template "${PROJECTPATHCONTAINMENT}"/metadata.desktop.cmake
echo "metadata.desktop file for containment was updated..."

cd ../plasmoid
intltool-merge --quiet --desktop-style . ../../plasmoid.metadata.desktop.template "${PROJECTPATHPLASMOID}"/metadata.desktop.cmake
echo "metadata.desktop file for plasmoid was updated..."

cd ../shell
intltool-merge --quiet --desktop-style . ../../shell.metadata.desktop.template "${PROJECTPATHSHELL}"/metadata.desktop.cmake
echo "metadata.desktop file for shell was updated..."

cd ../corona
intltool-merge --quiet --desktop-style . ../../latte-dock.desktop.template "${PROJECTPATHCORONA}"/latte-dock.desktop
echo "latte-dock.desktop file for corona was updated..."


