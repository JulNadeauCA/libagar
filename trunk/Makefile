#	$Csoft: Makefile,v 1.32 2003/02/08 00:38:18 vedge Exp $

SUBDIR=	 libfobj fobjcomp engine
SUBDIR+= cave

all: Makefile.config all-subdir
clean: clean-subdir
cleandir: clean-config cleandir-subdir
install: install-subdir
deinstall: deinstall-subdir
depend: prereq depend-subdir
regress: regress-subdir

prereq:
	(cd engine/compat && ${MAKE})
	(cd libfobj && ${MAKE})
	(cd fobjcomp && ${MAKE})

configure: .PHONY
	cat configure.in | manuconf > configure
	chmod 755 configure
	cvs commit -m "sync; rien" configure

clean-config: Makefile.config
	rm -fr config config.log

snap: cleandir
	sh mk/dist.sh

include mk/csoft.subdir.mk
