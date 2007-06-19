TOP=	../..
include ${TOP}/Makefile.config

LIB=		ag_core_monitor
LIB_SHARED=	Yes
LIB_STATIC=	No

SRCS=	leak.c monitor.c screenshot.c server.c timeouts.c uniconv.c \
	view_params.c widget_browser.c

CFLAGS+=${SDL_CFLAGS} ${JPEG_CFLAGS}

include ${TOP}/mk/build.lib.mk
