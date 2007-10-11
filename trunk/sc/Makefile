TOP=	..
include ${TOP}/Makefile.config

LIB=		ag_sc
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"d5385e50-a807-4991-972e-b9020d81aa27"

LIB_LINKS=	ag_core ag_gui

SRCS=	sc_math.c sc_matrix.c sc_vector.c sc_ivector.c sc_gaussj.c sc_lu.c \
	sc_matview.c sc_plotter.c sc.c sc_gui.c

CFLAGS+=-I.. ${SDL_CFLAGS} ${AGAR_CFLAGS}

include ${TOP}/mk/build.lib.mk
