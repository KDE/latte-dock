#!/bin/sh

cd "$(dirname $0)" # root of translatable sources
BASEDIR="$(pwd)"

BUGADDR="https://github.com/psifidotos/latte-dock/" # MSGID-Bugs

PROJECTCONTAINMENT="plasma_applet_org.kde.latte.containment" # project name
TEMPLATECONTCONTAINMENT="containment.metadata.desktop.template" # containment desktop template

PROJECTPLASMOID="plasma_applet_org.kde.latte.plasmoid" # project name
TEMPLATEPLASMOID="plasmoid.metadata.desktop.template" # plasmoid desktop template

PROJECTSHELL="plasma_shell_org.kde.latte.shell" # project name
TEMPLATESHELL="shell.metadata.desktop.template" # shell desktop template

PROJECTAPP="latte-dock" # project name
TEMPLATEAPP="latte-dock.desktop.template" # app desktop template

NOTIFYRC="lattedock.notifyrc.template" # notifyrc template

function ki18n_xgettext
{
    cd "$BASEDIR/$1"

    WDIR="." # working dir
    ROOT="../../"
    PROJECTNAME=$2 # project name
    TEMPLATE=$3 # desktop template
    PROJECTPATH="../../$1" # project path
    PROJECTPATH2= #extra project path
    TARGET="\e[0;32m$1\e[0m"

    if [ $4 ] ; then
        PROJECTPATH2="../../$4"
        TARGET="$TARGET and \e[0;32m$4\e[0m"
    else
        PROJECTPATH2=$PROJECTPATH # extra project path
    fi

    echo -e "-- Preparing rc files for $TARGET"
    find "${PROJECTPATH}" "${PROJECTPATH2}" -name '*.rc' -o -name '*.ui' -o -name '*.kcfg' | sort > "${WDIR}/rcfiles.list"
    xargs --no-run-if-empty --arg-file="${WDIR}/rcfiles.list" extractrc > "${WDIR}/rc.cpp"
    echo "rc.cpp" > "${WDIR}/infiles.list"
    echo -e "-- Done preparing rc files for $TARGET"

    echo -e "-- Extracting messages for $TARGET"
    find "${PROJECTPATH}" "${PROJECTPATH2}" -name '*.cpp' -o -name '*.h' -o -name '*.c' -o -name '*.qml' -o -name '*.qml.cmake' | sort >> "${WDIR}/infiles.list"

    xgettext --from-code=UTF-8 -C -kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 \
    -ktr2i18n:1 -kI18N_NOOP:1 -kI18N_NOOP2:1c,2  -kN_:1 -kaliasLocale -kki18n:1 -kki18nc:1c,2 \
    -kki18np:1,2 -kki18ncp:1c,2,3 --msgid-bugs-address="${BUGADDR}" --files-from=infiles.list \
    -D "${WDIR}" -o "${WDIR}/${PROJECTNAME}.pot" || \
    { echo "error while calling xgettext. aborting."; exit 1; }

    xgettext --from-code=UTF-8 --language=Desktop --join-existing --msgid-bugs-address="${BUGADDR}" \
    -k -kName -kGenericName -kComment \
    "${WDIR}/../desktop-templates/${TEMPLATE}" -o "${WDIR}/${PROJECTNAME}.pot" || \
    { echo "error while calling xgettext. aborting."; exit 1; }

    if [[ $1 == "app" ]] ; then
        xgettext --from-code=UTF-8 --language=Desktop --join-existing --msgid-bugs-address="${BUGADDR}" \
        -k -kName -kGenericName -kComment \
        "${WDIR}/../desktop-templates/${NOTIFYRC}" -o "${WDIR}/${PROJECTNAME}.pot" || \
        { echo "error while calling xgettext. aborting."; exit 1; }
    fi

    echo -e "-- Merging translations for $TARGET"
    catalogs=$(find "${WDIR}" -name '*.po')
    for cat in $catalogs; do
        echo "${cat}"
        msgmerge -o "${cat}.new" "${cat}" "${PROJECTNAME}.pot"
        mv "${cat}.new" "${cat}"
    done
    echo -e "-- Done merging translations for $TARGET"

    echo "-- Cleaning up"
    rm "rcfiles.list"
    rm "infiles.list"
    rm "rc.cpp"

    echo -e "-- Done translations for $TARGET\n\n"
}

ki18n_xgettext containment "$PROJECTCONTAINMENT" "$TEMPLATECONTCONTAINMENT"

ki18n_xgettext plasmoid    "$PROJECTPLASMOID"    "$TEMPLATEPLASMOID"

ki18n_xgettext app         "$PROJECTAPP"         "$TEMPLATEAPP"  shell


# The msg of shell package is merged with app
# ki18n_xgettext shell       "$PROJECTSHELL"       "$TEMPLATESHELL"


cd "$BASEDIR"
bash ./update-metadata.sh
