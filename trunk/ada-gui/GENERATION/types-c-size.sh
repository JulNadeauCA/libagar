#!/bin/sh

if [ $# -ne 1 ]
then
  echo "usage: typemap" 1>&2
  exit 111
fi

map="$1"

IFS="
"

for t in `cat "${map}" | awk -F: '{print $2}'`
do
  echo "  { \"$t\", sizeof ($t) },"
done
