#!/bin/sh

PROJECTPATHCONTAINMENT="../../containment" # containment path
PROJECTPATHPLASMOID="../../plasmoid" # plasmoid path
PROJECTPATHSHELL="../../shell" # shell path
BUGADDR="https://github.com/psifidotos/latte-dock/" # MSGID-Bugs


cd containment
intltool-merge --quiet --desktop-style . ../../containment.metadata.desktop.template "${PROJECTPATHCONTAINMENT}"/metadata.desktop.cmake
echo "metadata.desktop files for containment were updated..."

cd ../plasmoid
intltool-merge --quiet --desktop-style . ../../plasmoid.metadata.desktop.template "${PROJECTPATHPLASMOID}"/metadata.desktop.cmake
echo "metadata.desktop files for plasmoid were updated..."

cd ../shell
intltool-merge --quiet --desktop-style . ../../shell.metadata.desktop.template "${PROJECTPATHSHELL}"/metadata.desktop.cmake
echo "metadata.desktop files for shell were updated..."


