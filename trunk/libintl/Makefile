#	$Csoft: Makefile,v 1.1.1.1 2003/08/06 01:57:48 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

LIB=	intl
SRCS=	gettext.c textdomain.c
OBJS=	gettext.o textdomain.o

MAN3=	gettext.3
CATMAN3=gettext.cat3
PSMAN3=	gettext.ps3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
