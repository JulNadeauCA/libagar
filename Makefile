#	$Csoft: Makefile,v 1.30 2003/01/01 04:12:39 vedge Exp $

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
	(cd libfobj && ${MAKE})
	(cd fobjcomp && ${MAKE})

configure: .PHONY
	cat configure.in | manuconf > configure
	chmod 755 configure
	cvs commit -m "sync; rien" configure

clean-config: Makefile.config
	rm -fr config config.log Makefile.config

snap: cleandir
	sh mk/dist.sh

include mk/csoft.subdir.mk
