TOP=	..
include ${TOP}/Makefile.config

LIB=		ag_intl
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	gettext.c textdomain.c
#MAN3=	gettext.3

CFLAGS+=${SDL_CFLAGS}

include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
