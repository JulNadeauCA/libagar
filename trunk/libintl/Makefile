#	$Csoft: Makefile,v 1.3 2003/08/13 01:47:35 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

LIB=		intl
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	gettext.c textdomain.c
MAN3=	gettext.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
