#	$Csoft: Makefile,v 1.40 2004/03/10 16:58:29 vedge Exp $

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
	pax -rw -pa -L `find . -follow -type f -name '*.h' -print` ${INCLDIR}
	@if [ "${SRC}" != "" ]; then \
		cd ${SRC} && pax -rw -pa -L \
		    `find . -follow -type f -name '*.h' -print` \
		    ${INCLDIR}; \
	fi

deinstall-includes:
	rm -fR ${INCLDIR}

.PHONY: clean cleandir install deinstall depend regress
.PHONY: prereq configure clean-config release
.PHONY: install-includes deinstall-includes

include ${TOP}/mk/csoft.common.mk
include ${TOP}/mk/csoft.subdir.mk
