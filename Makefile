#	$Csoft: Makefile,v 1.9 2002/01/29 19:24:01 vedge Exp $

SUBDIR=	 libfobj fobjcomp fobjdump engine
SUBDIR+= geggy

all: Makefile.config all-subdir
clean: clean-subdir
cleandir: clean-config clean-subdir
install: install-subdir
deinstall: deinstall-subdir
depend: depend-subdir
regress: regress-subdir

configure: .PHONY
	cat configure.in | manuconf > configure
	chmod 755 configure

clean-config: Makefile.config
	rm -f config.h

include mk/csoft.subdir.mk
