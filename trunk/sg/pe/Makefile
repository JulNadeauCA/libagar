TOP=	../..
include ${TOP}/Makefile.config

LIB=		ag_sg_pe
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	pe.c pe_ode.c

CFLAGS+=${SDL_CFLAGS} ${AGAR_CFLAGS} ${ODE_CFLAGS}
CFLAGS+=${ALTIVEC_CFLAGS} ${SSE_CFLAGS} ${SSE2_CFLAGS} ${SSE3_CFLAGS}
CFLAGS+=-I..

include ${TOP}/mk/build.lib.mk
