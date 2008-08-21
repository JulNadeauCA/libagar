#!/bin/sh
# auto generated, do not edit

size_ada=`./ada_size "agar.core.object.checksum_alg_t"`
if [ $? -ne 0 ]; then exit 2; fi
size_c=`./c_size "enum ag_object_checksum_alg"`
if [ $? -ne 0 ]; then exit 2; fi

printf "%8d %8d %s -> %s\n" "${size_ada}" "${size_c}" "agar.core.object.checksum_alg_t" "enum ag_object_checksum_alg"

if [ ${size_ada} -ne ${size_c} ]
then
  echo "error: size mismatch" 1>&2
  exit 1
fi
