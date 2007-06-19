TOP=	../..
include ${TOP}/Makefile.config

LIB=		ag_sgimport
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	ply.c
CFLAGS+=${SDL_CFLAGS} ${AGAR_CFLAGS} -I..

include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
