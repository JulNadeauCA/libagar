TOP=	..
include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc

LIB=		ag_au
LIB_MAJOR=	5
LIB_MINOR=	0
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"26f09743-b62e-4c74-abaf-0a4339a2b5d9"
LIB_LINKS=	${CORE_LINKS_STATIC} portaudio sndfile

SRCS=	au.c au_wave.c au_dev_out.c au_dev_file.c au_dev_pa.c
MAN3=	AU.3

CFLAGS+=-D_AGAR_AU_INTERNAL ${MATH_CFLAGS} ${GETTEXT_CFLAGS} \
	${SNDFILE_CFLAGS} ${PORTAUDIO_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
