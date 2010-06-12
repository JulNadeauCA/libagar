TOP=..
include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

LIB=		ag_dev
LIB_MAJOR=	4
LIB_MINOR=	0
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"e0baea2d-5cb8-4a7a-9120-8352f36109e7"
LIB_LINKS=	${CORE_LINKS_STATIC} ${GUI_LINKS_STATIC}

SRCS=	dev.c browser.c screenshot.c server.c timeouts.c uniconv.c \
	view_params.c cpuinfo.c config.c object.c \
	classes.c

CFLAGS+=-D_AGAR_DEV_INTERNAL ${SDL_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS} \
	${GETTEXT_CFLAGS}

include ${TOP}/mk/build.lib.mk
