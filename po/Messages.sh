#!/bin/sh

BASEDIR="../.." # root of translatable sources
PROJECT="plasma_applet_org.kde.nowdock.containment" # project name
PROJECTPATH="../../containment" # project path
BUGADDR="https://github.com/psifidotos/nowdock-panel/" # MSGID-Bugs
WDIR="`pwd`/containment" # working dir

PROJECTPLASMOID="plasma_applet_org.kde.store.nowdock.plasmoid" # project name
PROJECTPATHPLASMOID="../../plasmoid" # project path
WDIRPLASMOID="`pwd`/plasmoid" # working di

echo "Preparing rc files for panel"

cd containment

# we use simple sorting to make sure the lines do not jump around too much from system to system
find "${PROJECTPATH}" -name '*.rc' -o -name '*.ui' -o -name '*.kcfg' | sort > "${WDIR}/rcfiles.list"
xargs --arg-file="${WDIR}/rcfiles.list" extractrc > "${WDIR}/rc.cpp"

# additional string for KAboutData
# echo 'i18nc("NAME OF TRANSLATORS","Your names");' >> "${WDIR}/rc.cpp"
# echo 'i18nc("EMAIL OF TRANSLATORS","Your emails");' >> "${WDIR}/rc.cpp"

intltool-extract --quiet --type=gettext/ini ../../containment.metadata.desktop.template

cat ../../containment.metadata.desktop.template.h >> ${WDIR}/rc.cpp

rm ../../containment.metadata.desktop.template.h

echo "Done preparing rc files for panel"
echo "Extracting messages for panel"

# see above on sorting
find "${PROJECTPATH}" -name '*.cpp' -o -name '*.h' -o -name '*.c' -o -name '*.qml' -o -name '*.qml.cmake' | sort > "${WDIR}/infiles.list"
echo "rc.cpp" >> "${WDIR}/infiles.list"

xgettext --from-code=UTF-8 -C -kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 \
	-ktr2i18n:1 -kI18N_NOOP:1 -kI18N_NOOP2:1c,2  -kN_:1 -kaliasLocale -kki18n:1 -kki18nc:1c,2 \
	-kki18np:1,2 -kki18ncp:1c,2,3 --msgid-bugs-address="${BUGADDR}" --files-from=infiles.list \
	-D "${BASEDIR}" -D "${WDIR}" -o "${PROJECT}.pot" || \
	{ echo "error while calling xgettext. aborting."; exit 1; }
echo "Done extracting messages for panel"

echo "Merging translations for panel"
catalogs=`find . -name '*.po'`
for cat in $catalogs; do
	echo "$cat"
	msgmerge -o "$cat.new" "$cat" "${WDIR}/${PROJECT}.pot"
	mv "$cat.new" "$cat"
done

intltool-merge --quiet --desktop-style . ../../containment.metadata.desktop.template "${PROJECTPATH}"/metadata.desktop.cmake

echo "Done merging translations for panel"
echo "Cleaning up"
rm "${WDIR}/rcfiles.list"
rm "${WDIR}/infiles.list"
rm "${WDIR}/rc.cpp"
echo "Done translations for panel" 

#---------------------- Plasmoid section ----------------#

#!/bin/sh

#BASEDIR=".." # root of translatable sources

echo "Preparing rc files for plasmoid"
cd ../plasmoid

# we use simple sorting to make sure the lines do not jump around too much from system to system
find "${PROJECTPATHPLASMOID}" -name '*.rc' -o -name '*.ui' -o -name '*.kcfg' | sort > "${WDIRPLASMOID}/rcfiles.list"
xargs --arg-file="${WDIRPLASMOID}/rcfiles.list" extractrc > "${WDIRPLASMOID}/rc.cpp"

intltool-extract --quiet --type=gettext/ini ../../plasmoid.metadata.desktop.template
cat ../../plasmoid.metadata.desktop.template.h >> ${WDIRPLASMOID}/rc.cpp
rm ../../plasmoid.metadata.desktop.template.h

echo "Done preparing rc files for plasmoid"
echo "Extracting messages for plasmoid"

# see above on sorting

find "${PROJECTPATHPLASMOID}" -name '*.cpp' -o -name '*.h' -o -name '*.c' -o -name '*.qml' -o -name '*.qml.cmake' | sort > "${WDIRPLASMOID}/infiles.list"
echo "rc.cpp" >> "${WDIRPLASMOID}/infiles.list"

xgettext --from-code=UTF-8 -C -kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 \
	-ktr2i18n:1 -kI18N_NOOP:1 -kI18N_NOOP2:1c,2  -kN_:1 -kaliasLocale -kki18n:1 -kki18nc:1c,2 \
	-kki18np:1,2 -kki18ncp:1c,2,3 --msgid-bugs-address="${BUGADDR}" --files-from=infiles.list \
	-D "${BASEDIR}" -D "${WDIRPLASMOID}" -o "${PROJECTPLASMOID}.pot" || \
	{ echo "error while calling xgettext. aborting."; exit 1; }
echo "Done extracting messages for plasmoid"

echo "Merging translations for plasmoid"
catalogs=`find . -name '*.po'`
for cat in $catalogs; do
	echo "$cat"
	msgmerge -o "$cat.new" "$cat" "${WDIRPLASMOID}/${PROJECTPLASMOID}.pot"
	mv "$cat.new" "$cat"
done

intltool-merge --quiet --desktop-style . ../../plasmoid.metadata.desktop.template "${PROJECTPATHPLASMOID}"/metadata.desktop.cmake

echo "Done merging translations for plasmoid"
echo "Cleaning up for plasmoid"
rm "${WDIRPLASMOID}/rcfiles.list"
rm "${WDIRPLASMOID}/infiles.list"
rm "${WDIRPLASMOID}/rc.cpp"
echo "Done" 




