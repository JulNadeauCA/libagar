#!/bin/sh

IFS="
"

echo "  -- auto generated - do not edit"
for t in `cat types-map.txt | awk -F: '{print $1}' | uniq | sort`
do
  tr_name=`echo $t | tr . _`
  tr_name=`echo $tr_name | tr -d ' '`
  t_nospace=`echo $t | tr -d ' '`
  cat <<EOF
  ${tr_name} : aliased string := "${t_nospace}";
EOF
done

cat <<EOF

  type type_t is record
    name : access string;
    size : natural;
  end record;
  type type_lookup_t is array (natural range <>) of type_t;

  types : aliased constant type_lookup_t := (
EOF

for t in `cat types-map.txt | awk -F: '{print $1}' | uniq | sort`
do
  tr_name=`echo $t | tr . _`
  tr_name=`echo $tr_name | tr -d ' '`
  t_nospace=`echo $t | tr -d ' '`
  cat <<EOF
    (${tr_name}'access, ${t_nospace}'size),
EOF
done
echo "  );"
echo
