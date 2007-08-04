TOP=..
include ${TOP}/Makefile.config

LIB=		ag_dev
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	browser.c monitor.c leak.c screenshot.c server.c timeouts.c uniconv.c \
	view_params.c widget_browser.c

CFLAGS+=${SDL_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS} \
	${PTHREADS_XOPEN_CFLAGS}

include ${TOP}/mk/build.lib.mk
