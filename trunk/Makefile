#	$Csoft: Makefile,v 1.3 2002/01/26 20:08:11 vedge Exp $

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

configure: configure.in
	cat configure.in |perl ~/src/csoft-mk/mkconf.pl > configure
	chmod 755 configure

include mk/csoft.subdir.mk
