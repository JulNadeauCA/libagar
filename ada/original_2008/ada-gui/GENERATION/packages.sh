#!/bin/sh

if [ $# -ne 1 ]
then
  echo 'usage: typemap' 1>&2
  exit 111
fi

map="$1"
awk -F: '{print $1}' < "${map}" | sed -E 's/\.[a-z_]*$//g' | sort -u
