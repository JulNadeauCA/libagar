#	$Csoft: Makefile,v 1.37 2003/08/06 04:11:22 vedge Exp $

SUBDIR=	 dencomp \
	 denex \
	 engine \
	 libintl \
	 cave \
	 po
	
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

.PHONY: clean cleandir install deinstall depend regress prereq clean-config snap configure

include mk/csoft.subdir.mk
