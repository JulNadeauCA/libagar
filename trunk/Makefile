#	$Csoft: Makefile,v 1.35 2003/06/21 06:50:07 vedge Exp $

SUBDIR=	 dencomp denex engine
SUBDIR+= cave
SUBDIR+= po

all: Makefile.config all-subdir
clean: clean-subdir
cleandir: clean-config cleandir-subdir
install: install-subdir
deinstall: deinstall-subdir
depend: prereq depend-subdir
regress: regress-subdir

prereq:
	(cd engine/compat && ${MAKE})
	(cd engine/error && ${MAKE})
	(cd engine/loader && ${MAKE})
	(cd dencomp && ${MAKE})

configure:
	cat configure.in | manuconf > configure
	chmod 755 configure
	cvs commit -m "sync; rien" configure

clean-config: Makefile.config
	rm -fr config config.log

snap: cleandir
	sh mk/dist.sh
	sh mk/agar-cvs.sh

.PHONY: clean cleandir install deinstall depend regress prereq clean-config snap

include mk/csoft.subdir.mk
