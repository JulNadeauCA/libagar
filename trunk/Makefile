#	$Csoft: Makefile,v 1.4 2002/01/27 11:53:26 vedge Exp $

SUBDIR=	 libfobj fobjcomp fobjdump engine
SUBDIR+= geggy

all: all-subdir
clean: clean-subdir
install: install-subdir
deinstall: deinstall-subdir

.BEGIN:
	@if [ ! -e "Makefile.config" ]; then \
	    echo "sh configure"; \
	    sh configure; \
	fi

configure: configure.in /home/vedge/src/csoft-mk/manuconf.pl
	cat configure.in |perl /home/vedge/src/csoft-mk/manuconf.pl > configure
	chmod 755 configure

include mk/csoft.subdir.mk
