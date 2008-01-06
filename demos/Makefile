TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	fixedres \
	objsystem \
	focusing \
	loader \
	table \
	textbox \
	widgets \
	maximized \
	objprops \
	unitconv \
	sockets \
	glview \
	gunpowder \
	cplusplus \
	doublebuf \
	plotting

all: all-subdir
clean: prereq clean-subdir
cleandir: prereq cleandir-subdir
deinstall: deinstall-subdir
depend: depend-subdir

install:
	@echo "*"
	@echo "* You probably don't want to install the demos on your system,"
	@echo "* they are meant to be executed from the source directory."
	@echo "*"
	@exit 1

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

proj:
	for D in ${SUBDIR}; do \
		echo "==> $$D"; \
		(cd $$D && ${MAKE} proj); \
	done

.PHONY: clean cleandir install deinstall depend configure configure-scripts prereq proj

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.subdir.mk
