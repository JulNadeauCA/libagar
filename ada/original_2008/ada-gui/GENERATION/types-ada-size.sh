#!/bin/sh

if [ $# -ne 2 ]
then
  echo "usage: typemap generics" 1>&2
  exit 111
fi

map="$1"
gen="$2"

IFS="
"

for pkg in `cat "${gen}"`
do
  tr_name=`echo ${pkg} | tr . _`

  cat <<EOF
  package gen_${tr_name} is new ${pkg} (child_type => integer);
EOF
done

echo

for t in `cat "${map}" | awk -F: '{print $1}' | uniq | sort`
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

for t in `cat "${map}" | awk -F: '{print $1}' | uniq | sort`
do
  tr_name=`echo $t | tr . _`
  tr_name=`echo $tr_name | tr -d ' '`
  t_nospace=`echo $t | tr -d ' '`
  t_pkgname=`echo $tr_name | awk '{ $NF = ""; sub( / $/, ""); print }'`
  t_basename=`echo $tr_name | awk -F. '{print $NF}'`

  # check if type is from generic package
  grep "${t_pkgname}" "${gen}"
  case $? in
    0) # generic
      cat <<EOF
    (${tr_name}'access, gen_${tr_name}.${t_basename}'size),
EOF
      ;;
    1) # not generic
      cat <<EOF
    (${tr_name}'access, ${t_nospace}'size),
EOF
      ;;
    *)
      echo "fatal: grep error" 1>&2
      exit 112
      ;;
  esac
done
echo "  );"
echo
