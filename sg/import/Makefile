TOP=	../..
include ${TOP}/Makefile.config

LIB=		ag_sg_import
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	ply.c

CFLAGS+=${SDL_CFLAGS} ${AGAR_CFLAGS}
CFLAGS+=${ALTIVEC_CFLAGS} ${SSE_CFLAGS} ${SSE2_CFLAGS} ${SSE3_CFLAGS}
CFLAGS+=-I..

include ${TOP}/mk/build.lib.mk
