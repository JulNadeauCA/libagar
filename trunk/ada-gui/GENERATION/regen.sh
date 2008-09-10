#!/bin/sh -x

./ada_size.sh types-map.txt generics.txt > ../UNIT_TESTS/ada_size.adb.tmp &&
  mv ../UNIT_TESTS/ada_size.adb.tmp ../UNIT_TESTS/ada_size.adb
./c_size.sh types-map.txt > ../UNIT_TESTS/c_size.c.tmp &&
  mv ../UNIT_TESTS/c_size.c.tmp ../UNIT_TESTS/c_size.c
