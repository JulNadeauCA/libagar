#	$Csoft: Makefile,v 1.28 2002/08/23 07:16:00 vedge Exp $

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
	cvs commit -m "sync; rien" configure

clean-config: Makefile.config
	rm -f engine/mcconfig.h
	rm -f config.log

snap: cleandir
	sh mk/dist.sh

include mk/csoft.subdir.mk
