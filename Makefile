#	$Csoft: Makefile,v 1.43 2004/04/25 02:11:01 vedge Exp $

TOP=	.
include ${TOP}/Makefile.config

SUBDIR=	 agar-config \
	 dencomp \
	 denex \
	 engine \
	 libintl \
	 po

all: all-subdir
clean: clean-subdir
cleandir: cleandir-config cleandir-subdir
install: install-subdir install-includes
deinstall: deinstall-subdir deinstall-includes
depend: prereq depend-subdir
regress: regress-subdir

prereq:
	(cd compat && ${MAKE})
	(cd engine/error && ${MAKE})
	(cd engine/loader && ${MAKE})
	(cd dencomp && ${MAKE})

configure:
	cat configure.in | manuconf > configure
	chmod 755 configure
	cvs commit -m "sync; rien" configure

cleandir-config:
	rm -fr config config.log

release: cleandir
	sh mk/dist.sh
	sh mk/agar-cvs.sh

install-includes:
	${INSTALL_INCL_DIR} ${INCLDIR}
	${SUDO} env \
	    INSTALL_INCL_DIR="${INSTALL_INCL_DIR}" \
	    INSTALL_INCL="${INSTALL_INCL}" \
	    ${FIND} . -follow -type d \! -name CVS \
	    -exec ${SH} mk/install-includes.sh "{}" "${INCLDIR}/{}" \;
	@if [ "${SRC}" != "" ]; then \
		(cd ${SRC} && ${SUDO} env \
		    INSTALL_INCL_DIR="${INSTALL_INCL_DIR}" \
		    INSTALL_INCL="${INSTALL_INCL}" \
		    ${FIND} . -follow -type d \! -name CVS \
		    -exec ${SH} mk/install-includes.sh "{}" \
		    "${INCLDIR}/{}" \;); \
	fi

deinstall-includes:
	${FIND} . -follow -type f -name '*.h' -print \
	    | ${AWK} '{print "${DEINSTALL_INCL} ${INCLDIR}/"$$1}' \
	    | ${SUDO} ${SH}
	@if [ "${SRC}" != "" ]; then \
		echo "${FIND} ${SRC} -follow -type f -name '*.h' -print \
		    | ${AWK} '{print "${DEINSTALL_INCL} ${INCLDIR}/"$$1}' \
		    | ${SUDO} ${SH}"; \
		(cd ${SRC} && ${FIND} . -follow -type f -name '*.h' -print \
		    | ${AWK} '{print "${DEINSTALL_INCL} ${INCLDIR}/"$$1}' \
		    | ${SUDO} ${SH}); \
	fi

.PHONY: clean cleandir install deinstall depend regress
.PHONY: prereq configure clean-config release
.PHONY: install-includes deinstall-includes

include ${TOP}/mk/csoft.common.mk
include ${TOP}/mk/csoft.subdir.mk
