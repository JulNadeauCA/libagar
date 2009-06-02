TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/dev/Makefile.inc

PROG=		agarrcsd
PROG_TYPE=	"DAEMON"
PROG_GUID=	"45170011-03CA-40f2-BA07-D954E3C61CA2"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS} ${DEV_LINKS}

SRCS=	agarrcsd.c user.c mkpath.c rcsinfo.c rcs.c fgetln.c
MAN8=	agarrcsd.8

CFLAGS+=${AGAR_CFLAGS} ${AGAR_NET_CFLAGS} ${AGAR_DEV_CFLAGS}
LIBS+=	${AGAR_LIBS} ${AGAR_NET_LIBS} ${AGAR_DEV_LIBS}

all: all-subdir
clean: clean-subdir
cleandir: cleandir-config cleandir-subdir
install: install-subdir install-includes
deinstall: deinstall-subdir deinstall-includes
depend: depend-subdir
regress: regress-subdir

configure:
	cat configure.in | mkconfigure > configure
	chmod 755 configure

cleandir-config:
	rm -fr config config.log

package: cleandir
	sh mk/dist.sh

release: cleandir
	sh mk/dist.sh commit

include ${TOP}/mk/build.prog.mk
include ${TOP}/mk/build.man.mk

.PHONY: clean cleandir install deinstall depend regress
.PHONY: configure clean-config release fastclean
.PHONY: install-includes deinstall-includes
