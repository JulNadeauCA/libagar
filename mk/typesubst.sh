#!/bin/sh
#
# Substitute %%sizeof(TYPE)%% and %%const(TYPE)%% in input text according
# to the specified type map file.
#
# Examples of standard BSDBuild tests which output a type map include
# agar.types, agar-core.types.
#

nl='
'
_src="$1"
_types="$2"
_srctmp="$_src.tmp"

if [ "$_src" = "" -o "$_types" = "" ]; then
	echo "Usage: typesubst.sh srcfile typefile"
	exit 1
fi
if [ ! -e "$_types" ]; then
	echo "Missing $_types (needed by $_src)"
	exit 1
fi
cp -f $_src $_srctmp
if [ $? != 0 ]; then exit 1; fi
bbmk_save_IFS=$IFS
IFS=$nl
for LINE in `cat $_types`; do
	if echo "$LINE" | grep -q '^#'; then
		continue;
	fi
	if echo "$LINE" | grep -q '^$'; then
		continue;
	fi
	mytype=`echo "$LINE" | awk -F: '{print $1}'`;
	mysize=`echo "$LINE" | awk -F: '{print $2}'`;
	mypat="%%sizeof($mytype)%%";
	if grep -q "$mypat" $_src; then
		sed "s/$mypat/$mysize/" $_srctmp > $_srctmp.tmp$$
		if [ $? != 0 ]; then exit 1; fi
		mv -f $_srctmp.tmp$$ $_srctmp
		if [ $? != 0 ]; then exit 1; fi
	fi
	mypat="%%const($mytype)%%";
	if grep -q "$mypat" $_src; then
		sed "s/$mypat/$mysize/" $_srctmp > $_srctmp.tmp$$
		if [ $? != 0 ]; then exit 1; fi
		mv -f $_srctmp.tmp$$ $_srctmp
		if [ $? != 0 ]; then exit 1; fi
	fi
done
IFS=$bbmk_save_IFS

cat $_srctmp
if [ $? != 0 ]; then exit 1; fi
rm -f $_srctmp

exit 0
