#	$Csoft: Makefile,v 1.24 2002/06/04 22:50:10 darkroot Exp $

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
	(cd fobjcomp && ${MAKE})

configure: .PHONY
	cat configure.in | manuconf > configure
	chmod 755 configure

clean-config: Makefile.config
	rm -f engine/mcconfig.h
	rm -f config.log

include mk/csoft.subdir.mk
