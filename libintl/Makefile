#	$Csoft: Makefile,v 1.4 2004/03/10 16:58:40 vedge Exp $

TOP=	..
include ${TOP}/Makefile.config

LIB=		bsd_intl
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	gettext.c textdomain.c
MAN3=	gettext.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
include ${TOP}/mk/csoft.man.mk
