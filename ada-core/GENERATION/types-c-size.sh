#!/bin/sh

IFS="
"

echo "  /* auto generated - do not edit */"
for t in `cat types-map.txt | awk -F: '{print $2}'`
do
  echo "  { \"$t\", sizeof ($t) },"
done
