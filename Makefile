#	$Csoft: Makefile,v 1.48 2005/02/03 11:22:11 vedge Exp $

TOP=	.
include ${TOP}/Makefile.config

SUBDIR=	 agar-config \
	 compat \
	 core \
	 gui \
	 vg \
	 rg \
	 game \
	 mat \
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
	@if [ ! -e "agar" ]; then ln -s . agar; fi

configure:
	cat configure.in | manuconf > configure
	chmod 755 configure

cleandir-config:
	rm -fr config config.log

package: cleandir
	sh mk/dist.sh

release: cleandir
	sh mk/dist.sh commit

fastclean:
	find . -type f -and \( -name \*.o -or -name \*.lo -or \
	          -name \*.la -or -name \*.a -or \
	          -name libtool -or -name \*.out \
	          -name .depend -or -name config.log -or \
	          -name stdout.txt -or -name stderr.txt \) \
		  -exec rm -f {} \;
	find . -type d -and -name .libs -exec rm -fR {} \;

install-includes:
	${SUDO} ${INSTALL_INCL_DIR} ${INCLDIR}
	${SUDO} env \
	    INSTALL_INCL_DIR="${INSTALL_INCL_DIR}" \
	    INSTALL_INCL="${INSTALL_INCL}" \
	    ${FIND} . -type d \! -name CVS \
	    -exec ${SH} mk/install-includes.sh "{}" "${INCLDIR}/{}" \;
	@if [ "${SRC}" != "" ]; then \
		(cd ${SRC} && ${SUDO} env \
		    INSTALL_INCL_DIR="${INSTALL_INCL_DIR}" \
		    INSTALL_INCL="${INSTALL_INCL}" \
		    ${FIND} . -type d \! -name CVS \
		    -exec ${SH} mk/install-includes.sh "{}" \
		    "${INCLDIR}/{}" \;); \
		echo "${INSTALL_INCL} gui.h ${INCLPDIR}/gui.h"; \
		${SUDO} ${INSTALL_INCL} ${SRC}/gui/gui.h ${INCLPDIR}/gui.h; \
		echo "${INSTALL_INCL} core_pub.h ${INCLPDIR}/core.h"; \
		${SUDO} ${INSTALL_INCL} ${SRC}/core/core_pub.h ${INCLPDIR}/core.h; \
	else \
		echo "${INSTALL_INCL} gui/gui.h ${INCLPDIR}/gui.h"; \
		${SUDO} ${INSTALL_INCL} gui/gui.h ${INCLPDIR}/gui.h; \
		echo "${INSTALL_INCL} core/core_pub.h ${INCLPDIR}/core.h"; \
		${SUDO} ${INSTALL_INCL} core/core_pub.h ${INCLPDIR}/core.h; \
	fi

deinstall-includes:
	${FIND} . -type f -name '*.h' -print \
	    | ${AWK} '{print "${DEINSTALL_INCL} ${INCLDIR}/"$$1}' \
	    | ${SUDO} ${SH}
	@if [ "${SRC}" != "" ]; then \
		echo "${FIND} ${SRC} -type f -name '*.h' -print \
		    | ${AWK} '{print "${DEINSTALL_INCL} ${INCLDIR}/"$$1}' \
		    | ${SUDO} ${SH}"; \
		(cd ${SRC} && ${FIND} . -type f -name '*.h' -print \
		    | ${AWK} '{print "${DEINSTALL_INCL} ${INCLDIR}/"$$1}' \
		    | ${SUDO} ${SH}); \
	fi

.PHONY: clean cleandir install deinstall depend regress
.PHONY: prereq configure clean-config release fastclean
.PHONY: install-includes deinstall-includes

include ${TOP}/mk/csoft.common.mk
include ${TOP}/mk/csoft.subdir.mk
