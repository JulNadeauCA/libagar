#!/bin/sh

if [ $# -ne 2 ]
then
  echo "usage: typemap generics" 1>&2
  exit 111
fi

map="$1"
gen="$2"

(cat <<EOF
-- auto generated, do not edit

with ada.text_io;
with ada.command_line;

with agar;
with agar.gui;
EOF
) || exit 112

for line in `./packages.sh ${map} || exit 112`
do
  echo "with $line;" || exit 112
done

(cat <<EOF

procedure ada_size is
  package io renames ada.text_io;
  package cmdline renames ada.command_line;

EOF
) || exit 112

./types-ada-size.lua "${map}" "${gen}" || exit 112

(cat <<EOF
  procedure find (name : string) is
  begin
    for index in types'range loop
      if types (index).name.all = name then
        io.put_line (natural'image (types (index).size));
        return;
      end if;
    end loop;
    raise program_error with "fatal: unknown ada type";
  end find;

begin
  if cmdline.argument_count /= 1 then
    raise program_error with "fatal: incorrect number of args";
  end if;
  find (cmdline.argument (1));
end ada_size;
EOF
) || exit 112
