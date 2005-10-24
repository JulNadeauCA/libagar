#	$Csoft: Makefile,v 1.49 2005/09/11 02:33:57 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	gamemenu \
	glview \
	objmgr \
	focusing \
	vectordwg \
	loader \
	table \
	textbox \
	widgets \
	nopadding

all: configure all-subdir
clean: prereq clean-subdir
cleandir: prereq cleandir-subdir
install: install-subdir
deinstall: deinstall-subdir
depend: depend-subdir

configure:
	for D in ${SUBDIR}; do \
		echo "==> $$D"; \
		(cd $$D && ./configure); \
	done

configure-scripts:
	for D in ${SUBDIR}; do \
		echo "==> $$D"; \
		(cd $$D && ${MAKE} configure); \
	done

prereq:
	for D in ${SUBDIR}; do \
		echo "==> $$D"; \
		touch $$D/Makefile.config; \
	done

.PHONY: clean cleandir install deinstall depend configure configure-scripts prereq

include ${TOP}/mk/csoft.common.mk
include ${TOP}/mk/csoft.subdir.mk
