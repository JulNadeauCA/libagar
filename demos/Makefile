#	$Csoft: Makefile,v 1.49 2005/09/11 02:33:57 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	gamemenu \
	glview \
	objmgr \
	focusing \
	vectordwg \
	loader

all: prereq all-subdir
clean: prereq clean-subdir
cleandir: prereq cleandir-subdir
install: install-subdir
deinstall: deinstall-subdir
depend: depend-subdir

prereq:
	for D in ${SUBDIR}; do \
		(cd $$D && touch Makefile.config); \
	done

.PHONY: clean cleandir install deinstall depend

include ${TOP}/mk/csoft.common.mk
include ${TOP}/mk/csoft.subdir.mk
