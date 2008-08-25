TOP=..
include ${TOP}/Makefile.config

LIB=		ag_dev
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"e0baea2d-5cb8-4a7a-9120-8352f36109e7"
LIB_LINKS=	ag_core_static ag_gui_static pthreads SDL opengl freetype

SRCS=	dev.c browser.c screenshot.c server.c timeouts.c uniconv.c \
	view_params.c widget_browser.c cpuinfo.c config.c object.c \
	classes.c

CFLAGS+=-I.. ${SDL_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS} ${GETTEXT_CFLAGS}

include ${TOP}/mk/build.lib.mk
