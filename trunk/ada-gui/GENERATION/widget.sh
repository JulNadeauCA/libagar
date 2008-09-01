#!/bin/sh

if [ $# -ne 1 ]
then
  echo "usage: name" 1>&2
  exit 111
fi
name="$1"

pkg="agar.gui.widget.$name"
pkg_file="agar-gui-widget-$name"
ads_file="$pkg_file.ads"
apk_file="$pkg_file.apk"

if [ -f "${ads_file}" ] || [ -f "${apk_file}" ]
then
  echo "widget already exists" 1>&2
  exit 112
fi

(cat <<EOF
package $pkg is

end $pkg;
EOF
) > "${ads_file}"

(cat <<EOF
$pkg $ads_file
EOF
) > "${apk_file}"
