TOP=	../..
include ${TOP}/Makefile.config

LIB=		ag_sg_pe
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"246f705a-2082-482a-aa31-5e7d43ea0605"

SRCS=	pe.c pe_ode.c

CFLAGS+=${SDL_CFLAGS} ${AGAR_CFLAGS} ${ODE_CFLAGS}
CFLAGS+=${ALTIVEC_CFLAGS} ${SSE_CFLAGS} ${SSE2_CFLAGS} ${SSE3_CFLAGS}
CFLAGS+=-I../..

include ${TOP}/mk/build.lib.mk
