#!/bin/sh
#
# Public domain
#
# Install the manual pages ${MAN} and ${CATMAN} into ${MANDIR} and
# ${CATMANDIR}, respectively. ${CATMAN} is optional.
#

for F in ${MAN} x; do
	if [ "$F" = "x" ]; then continue; fi
	if [ -e "$F" ]; then
		echo "${INSTALL_DATA} $F ${MANDIR}"
		${INSTALL_DATA} $F ${DESTDIR}${MANDIR}
		if [ $? != 0 ]; then
			exit 1;
		fi
	else
		echo "* Skipping: $F"
	fi
done

if [ "${NOCATMAN}" != "yes" ]; then
	for F in ${CATMAN} x; do
		if [ "$F" = "x" ]; then continue; fi
		if [ -e "$F" ]; then
			CAT=`echo $F | sed 's/.1$$/.cat1/'`
			echo "${INSTALL_DATA} $CAT ${CATMANDIR}"
			${INSTALL_DATA} $CAT ${DESTDIR}${CATMANDIR}
			if [ $? != 0 ]; then
				exit 1;
			fi
		else
			echo "* Skipping: $F"
		fi
	done
fi
