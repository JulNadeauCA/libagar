#	$Csoft: Makefile,v 1.2 2002/01/26 01:37:22 vedge Exp $

SUBDIR=	 libfobj fobjcomp fobjdump engine
SUBDIR+= geggy

all: all-subdir
clean: clean-subdir clean-config
install: install-subdir
deinstall: deinstall-subdir

clean-config:
	rm -f Makefile.config

.BEGIN:
	@if [ ! -e "Makefile.config" ]; then \
	    echo "sh configure"; \
	    sh configure; \
	fi

configure: configure.in
	cat configure.in |perl ~/src/csoft-mk/mkconf.pl > configure
	chmod 755 configure

include mk/csoft.subdir.mk
