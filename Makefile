#	$Csoft: Makefile,v 1.25 2002/08/18 20:28:59 vedge Exp $

SUBDIR=	 libfobj fobjcomp fobjdump engine
SUBDIR+= cave


all: Makefile.config all-subdir
clean: clean-subdir
cleandir: clean-config cleandir-subdir
install: install-subdir
deinstall: deinstall-subdir
depend: prereq depend-subdir
regress: regress-subdir

prereq:
	(cd libfobj && ${MAKE})
	(cd compat && ${MAKE})
	(cd fobjcomp && ${MAKE})

configure: .PHONY
	cat configure.in | manuconf > configure
	chmod 755 configure

clean-config: Makefile.config
	rm -f engine/mcconfig.h
	rm -f config.log

include mk/csoft.subdir.mk
