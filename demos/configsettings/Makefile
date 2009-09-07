TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

PROJECT=	"configsettings"
PROG_TYPE=	"GUI"
PROG_GUID=	"21388f56-8e8d-4890-b4b0-b07b0bd757e2"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS}

PROG=	configsettings
SRCS=	configsettings.c

CFLAGS+=${AGAR_CFLAGS}
LIBS+=	${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
