#	$Csoft: Makefile,v 1.6 2002/01/28 05:07:40 vedge Exp $

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

configure: configure.in
	cat configure.in |perl /home/vedge/src/csoft-mk/manuconf.pl > configure
	chmod 755 configure

include mk/csoft.subdir.mk
