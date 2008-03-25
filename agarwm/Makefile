TOP=	..
include Makefile.config

PROG=		agarwm
PROG_TYPE=	"GUI"
PROG_GUID=	""
PROG_LINKS=	ag_core ag_gui ag_net ag_dev

SRCS=	agarwm.c client.c
#MAN1=	agarwm.1

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
