TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	agardb \
	agar-disasm \
	mapedit \
	rgedit \
	sgedit \
	skedit \
	vgedit

# For regenerating gui/*_data.h.
#SUBDIR+=bundlefont bundlecss

all: all-subdir
clean: prereq clean-subdir
cleandir: prereq cleandir-subdir cleandir-cache
deinstall: deinstall-subdir
depend: depend-subdir
configure: configure-subdir

install:
	for D in ${SUBDIR}; do \
		echo "==> $$D"; \
		(cd $$D && ${MAKE} install); \
	done

cleandir-cache:
	rm -fR .cache


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

.PHONY: clean cleandir cleandir-cache install deinstall depend configure prereq proj

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.subdir.mk
