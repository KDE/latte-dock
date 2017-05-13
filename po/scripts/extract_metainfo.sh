#!/bin/bash

#l10nscripts=`dirname $0`
ASMETAINFOITS="as-metainfo.its"

podir="../metainfo"
metainfo_file="../../app/org.kde.latte-dock.appdata.xml.cmake"

# first, strip translation from project metadata file
tmpxmlfile=$(mktemp)
xmlstarlet ed -d "//*[@xml:lang]" $metainfo_file > $tmpxmlfile

# get rid of the .xml extension and dirname
metainfo_file_basename=$(basename $metainfo_file)
dataname="${metainfo_file_basename%.xml*}"

# create pot file foo.[appdata|metadata].xml -> foo.[appdata|metadata].pot
itstool -i $ASMETAINFOITS -o $podir/$dataname.pot $tmpxmlfile
esc_tmpxmlfile=$(echo $tmpxmlfile|sed -e 's/[]\/()$*.^|[]/\\&/g')
sed -i "/^#:/s/$esc_tmpxmlfile/$metainfo_file_basename/" $podir/$dataname.pot

tmpmofiles=""
tmpdir=$(mktemp -d)
  
echo -e "-- Update translation strings for .po files in metainfo"
# find po files
catalogs=$(find "${podir}" -name '*.po')
for pofile in $catalogs; do
  # get language-id
  echo "${pofile}"
  msgmerge -o "${pofile}.new" "${pofile}" "$podir/$dataname.pot"
  mv "${pofile}.new" "${pofile}"
done
echo -e "-- Done updating translation strings for .po files in metainfo"

# cleanup
rm -rf $tmpdir
rm $tmpxmlfile

