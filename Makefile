#	$Csoft: Makefile,v 1.5 2002/01/28 04:21:50 vedge Exp $

SUBDIR=	 libfobj fobjcomp fobjdump engine
SUBDIR+= geggy

all: Makefile.config all-subdir
clean: clean-subdir
cleandir: clean-config clean-subdir
install: install-subdir
deinstall: deinstall-subdir

Makefile.config config.h: configure
	sh configure

clean-config: Makefile.config
	rm -f Makefile.config config.h

configure: configure.in /home/vedge/src/csoft-mk/manuconf.pl
	cat configure.in |perl /home/vedge/src/csoft-mk/manuconf.pl > configure
	chmod 755 configure

include mk/csoft.subdir.mk
