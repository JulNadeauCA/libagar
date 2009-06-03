TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/dev/Makefile.inc

PROJECT=	"objsystem"
PROG_TYPE=	"GUI"
PROG_GUID=	"85CA41FA-DD75-4136-8200-3AF5F3D7B6AF"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS} ${DEV_LINKS}

PROG=	objsystem
SRCS=	objmgr.c animal.c mammal.c

CFLAGS+=${AGAR_DEV_CFLAGS} ${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_DEV_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
