#	$Csoft: Makefile,v 1.1 2003/06/14 11:28:04 vedge Exp $

TOP=	../..
include ${TOP}/Makefile.config

LIB=	unicode
SRCS=	unicode.c ucslcpy.c ucslcat.c ucssep.c
OBJS=	unicode.o ucslcpy.o ucslcat.o ucssep.o

CFLAGS+=${SDL_CFLAGS} ${GETTEXT_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
