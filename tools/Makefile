TOP=	..
include ${TOP}/Makefile.config

SUBDIR=	agar-bench \
	agar-disasm \
	agarpaint \
	agarrcsd \
	dencomp \
	denuncomp

#SUBDIR+=bundlefont

all: all-subdir
clean: prereq clean-subdir
cleandir: prereq cleandir-subdir cleandir-cache
deinstall: deinstall-subdir
depend: depend-subdir

install:
	for D in ${SUBDIR}; do \
		echo "==> $$D"; \
		(cd $$D && ${MAKE} install); \
	done

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

.PHONY: clean cleandir cleandir-cache install deinstall depend configure configure-scripts prereq proj

include ${TOP}/mk/build.common.mk
include ${TOP}/mk/build.subdir.mk
