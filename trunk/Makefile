#	$Csoft: Makefile,v 1.7 2002/01/28 05:22:22 vedge Exp $

SUBDIR=	 libfobj fobjcomp fobjdump engine
SUBDIR+= geggy

all: Makefile.config all-subdir
clean: clean-subdir
cleandir: clean-config clean-subdir
install: install-subdir
deinstall: deinstall-subdir

clean-config: Makefile.config
	rm -f config.h

include mk/csoft.subdir.mk
