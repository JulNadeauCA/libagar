TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	fixedres \
	objmgr \
	focusing \
	loader \
	table \
	textbox \
	widgets \
	maximized \
	objprops \
	unitconv

SUBDIR+=sgview \
	linear \
	lorenz

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

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.subdir.mk
