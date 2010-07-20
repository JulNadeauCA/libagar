#!/bin/sh
#
# Public domain
#
# Install the manual pages ${MAN} and ${CATMAN} into ${MANDIR} and
# ${CATMANDIR}, respectively. ${CATMAN} is optional.
#

for F in ${MAN}; do
	echo "${INSTALL_DATA} $F ${MANDIR}"
	${INSTALL_DATA} $F ${MANDIR}
	if [ $? != 0 ]; then
		exit 1;
	fi
done

if [ "${NOCATMAN}" != "yes" ]; then
	for F in ${CATMAN} ignore; do
		if [ "$F" = "ignore" ]; then continue; fi
		CAT=`echo $F | sed 's/.1$$/.cat1/'`
		echo "${INSTALL_DATA} $CAT ${CATMANDIR}"
		${INSTALL_DATA} $CAT ${CATMANDIR}
		if [ $? != 0 ]; then
			exit 1;
		fi
	done
fi
