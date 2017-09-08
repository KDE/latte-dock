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

# find po files
catalogs=$(find "${podir}" -name '*.po')
for pofile in $catalogs; do
  # get language-id 
  lang=$(echo $(basename $pofile)|cut -d/ -f2)

  # generate mo files (need to be named after their language)
  mofile="$tmpdir/$lang.mo"
  msgfmt $pofile -o $mofile

  tmpmofiles="$tmpmofiles $mofile"
done

if [ -n "$tmpmofiles" ]; then
  # recreate file, using the untranslated temporary data and the translation
  itstool -i $ASMETAINFOITS -j $tmpxmlfile -o $metainfo_file $tmpmofiles
  echo -e "$metainfo_file_basename file for \e[0;32mappstream\e[0m was updated..."
fi

# cleanup
rm -rf $tmpdir
rm $tmpxmlfile

