#	$Csoft: Makefile,v 1.11 2002/02/01 03:55:42 vedge Exp $

SUBDIR=	 libfobj fobjcomp fobjdump engine
SUBDIR+= geggy

all: Makefile.config all-subdir
clean: clean-subdir
cleandir: clean-config clean-subdir
install: install-subdir
deinstall: deinstall-subdir
depend: prereq depend-subdir
regress: regress-subdir

prereq:
	(cd libfobj && ${MAKE})
	(cd fobjcomp && ${MAKE})

configure: .PHONY
	cat configure.in | manuconf > configure
	chmod 755 configure

clean-config: Makefile.config
	rm -f engine/config.h

include mk/csoft.subdir.mk
