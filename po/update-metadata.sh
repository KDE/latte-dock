#!/bin/bash

cd "$(dirname $0)" # root of translatable sources
BASEDIR="$(pwd)"

cd "$BASEDIR/.."

PROJECTCONTAINMENT="$(pwd)/containment/metadata.desktop.cmake" # containment path
PROJECTPLASMOID="$(pwd)/plasmoid/metadata.desktop.cmake" # plasmoid path
PROJECTSHELL="$(pwd)/shell/metadata.desktop.cmake" # shell path
PROJECTAPP="$(pwd)/app/org.kde.latte-dock.desktop.cmake" # app path
NOTIFYRC="$(pwd)/app/lattedock.notifyrc" # global notify config
SEPARATORAPPLET="$(pwd)/applets/separator/metadata.desktop.cmake"
SPACERAPPLET="$(pwd)/applets/spacer/metadata.desktop.cmake"

function generate_desktop_file
{
    cd "$BASEDIR/$1"
    
    EXTRADIR=""
    
    if [[ $1 == applets/* ]] ; then
        EXTRADIR=../
    fi     

    LINGUAS=$(ls | grep ".*.po$" | xargs --no-run-if-empty --max-args=1 basename -s .po)
    echo $LINGUAS > LINGUAS

    # msgfmt first reads the ‘LINGUAS’ file under directory,
    # and then processes all ‘.po’ files listed there
    msgfmt --desktop --template="$EXTRADIR../desktop-templates/$2" -d . -o "$3"
    rm "LINGUAS"

    echo -e "$(basename $2 .template) file for \e[0;32m$1\e[0m was updated..."
}

generate_desktop_file containment containment.metadata.desktop.template "$PROJECTCONTAINMENT"

generate_desktop_file plasmoid plasmoid.metadata.desktop.template "$PROJECTPLASMOID"

generate_desktop_file shell shell.metadata.desktop.template "$PROJECTSHELL"

generate_desktop_file app org.kde.latte-dock.desktop.template "$PROJECTAPP"

generate_desktop_file app lattedock.notifyrc.template "$NOTIFYRC"

generate_desktop_file applets/separator separator.metadata.desktop.template "$SEPARATORAPPLET"

generate_desktop_file applets/spacer spacer.metadata.desktop.template "$SPACERAPPLET"

#update the appstream metadata file
cd "$BASEDIR/scripts"
bash ./update_metainfo.sh
