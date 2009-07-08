TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	fixedres \
	objsystem \
	focusing \
	loader \
	table \
	textbox \
	themes \
	maximized \
	objprops \
	unitconv \
	sockets \
	glview \
	cplusplus \
	plotting \
	scrollbar \
	keyevents \
	customwidget \
	windows \
	threads \
	minimal \
	textdlg \
	scrollview \
	console \
	chinese

all: all-subdir
clean: prereq clean-subdir
cleandir: prereq cleandir-subdir cleandir-cache
deinstall: deinstall-subdir
depend: depend-subdir

install:
	@echo "*"
	@echo "* You probably don't want to install the demos on your system,"
	@echo "* they are meant to be executed from the source directory."
	@echo "*"
	@exit 1

configure:
	if [ ! -e ".cache" ]; then \
		echo "mkdir .cache"; \
		mkdir .cache; \
	fi
	for D in ${SUBDIR}; do \
		echo "==> $$D"; \
		(cd $$D && ./configure --cache=../.cache); \
	done

cleandir-cache:
	rm -fR .cache

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
		(cd $$D && touch Makefile.config && ${MAKE} proj); \
	done

proj-vs2005:
	for D in ${SUBDIR}; do \
		echo "==> $$D"; \
		(cd $$D && unzip ProjectFiles/vs2005-windows.zip); \
	done

.PHONY: clean cleandir cleandir-cache install deinstall depend configure configure-scripts prereq proj proj-vs2005

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.subdir.mk
