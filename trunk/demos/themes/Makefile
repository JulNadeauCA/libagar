TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/dev/Makefile.inc

PROJECT=	"themes"
PROG_TYPE=	"GUI"
PROG_GUID=	"11D6C9FF-522E-43ed-B3EB-92A2C636CCA7"
PROG_LINKS=	${CORE_LINKS} ${GUI_LINKS} ${DEV_LINKS}

PROG=	themes
SRCS=	themes.c mytheme.c

CFLAGS+=${AGAR_DEV_CFLAGS} ${AGAR_CFLAGS} -I.
LIBS+=	${AGAR_DEV_LIBS} ${AGAR_LIBS}

all: all-subdir ${PROG}

configure: configure.in
	cat configure.in | mkconfigure > configure
	chmod 755 configure

.PHONY: configure

include ${TOP}/mk/build.prog.mk
