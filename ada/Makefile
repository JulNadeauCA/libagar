TOP=	..

SUBDIR=	core \
	gui

all: all-subdir
clean: clean-subdir
cleandir: cleandir-subdir
install: all install-subdir
deinstall: deinstall-subdir
depend: depend-subdir
regress: regress-subdir

.PHONY: clean cleandir install deinstall depend regress

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.subdir.mk
