TOP=	..
include ${TOP}/Makefile.config

LIB=		ag_sc
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	sc_math.c sc_matrix.c sc_vector.c sc_ivector.c sc_gaussj.c sc_lu.c \
	sc_matview.c sc_plotter.c sc.c sc_gui.c

CFLAGS+=${SDL_CFLAGS} ${AGAR_CFLAGS}

include ${TOP}/mk/csoft.lib.mk
