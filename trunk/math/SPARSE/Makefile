TOP=	../..
include ${TOP}/Makefile.config

LIB=		SPARSE
LIB_INSTALL=	No
LIB_SHARED=	Yes
LIB_GUID=	"6051cbce-2498-406d-ae46-cf6ac89d05cf"
LIB_LINKS=	ag_core_static ag_gui_static \
		pthreads SDL opengl freetype

SRCS=	spallocate.c \
	spbuild.c \
	spfactor.c \
	spoutput.c \
	spsolve.c \
	sputils.c \
	spedacious.c

CFLAGS+=${SDL_CFLAGS} ${PTHREADS_XOPEN_CFLAGS} ${ALTIVEC_CFLAGS} ${SSE_CFLAGS} \
	${SSE2_CFLAGS} ${SSE3_CFLAGS} ${GLU_CFLAGS} ${MATH_CFLAGS} \
	-D_M_INTERNAL

include ${TOP}/mk/build.lib.mk
