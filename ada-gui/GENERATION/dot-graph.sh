#!/bin/sh

cat <<EOF
digraph agar_gui {
  graph [
    splines = true
    overlap = false
    rankdir = LR
    margin = "0,0"
    fontpath = "/usr/local/lib/X11/fonts/misc"
    fontname = "6x10.pcf.gz"
  ];
  node [
    fontname = "6x10.pcf.gz"
    fontsize = 9
    margin = "0.1,0"
  ];
  edge [
    penwidth = 0.5
  ];

EOF

for file in *.ads
do
  deps=`grep -h '^with agar' $file | awk '{print $2}'`
  for dep in ${deps}
  do
    mod_file=`echo $file | sed 's/\.ads//g' | sed 's/-/_/g'`
    mod_dep=`echo $dep | sed 's/;//g' | sed 's/\./_/g'`
    echo "  $mod_file -> $mod_dep;"
  done
done

echo "}"
