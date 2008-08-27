#!/bin/sh

if [ $# -ne 1 ]
then
  echo "usage: type-map" 1>&2
  exit 111
fi
type_map="$1"

IFS="
"

for line in `cat "${type_map}"`
do
  type_ada=`echo $line | awk -F: '{print $1}'`
  type_c=`echo $line | awk -F: '{print $2}'`

  type_ada_mod=`echo $type_ada | tr '.' '_'`
  file_name="size_${type_ada_mod}.t"

  (cat << EOF
#!/bin/sh
# auto generated, do not edit

size_ada=\`./ada_size "${type_ada}"\`
if [ \$? -ne 0 ]; then exit 2; fi
size_c=\`./c_size "${type_c}"\`
if [ \$? -ne 0 ]; then exit 2; fi

printf "%8d %8d %s -> %s\n" "\${size_ada}" "\${size_c}" "${type_ada}" "${type_c}"

if [ \${size_ada} -ne \${size_c} ]
then
  echo "error: size mismatch" 1>&2
  exit 1
fi
EOF
) > "${file_name}"
  chmod +x "${file_name}"
done
