TOP=..
include ${TOP}/Makefile.config

LIB=		ag_gi
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"d32d0804-fed3-405c-9484-5e4afe24e237"
LIB_LINKS=	ag_core_static \
		SDL opengl

SRCS=	gi.c gi_surface.c \
	gi_gl.c gi_sdl.c

CFLAGS+=-I.. ${SDL_CFLAGS} ${OPENGL_CFLAGS} ${X11_CFLAGS} \
	${PTHREADS_XOPEN_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
