TOP=..
include ${TOP}/Makefile.config

SUBDIR=	monitor \
	loaders

LIB=		ag_core
LIB_SHARED=	Yes
LIB_STATIC=	No
SHARESRC=	core-icons.den

SRCS=	config.c core.c error.c event.c math.c iconmgr.c \
	object.c objmgr.c prop.c rcs.c timeout.c typesw.c view.c
MAN3=	AG_Intro.3 AG_Core.3 AG_Event.3 AG_Object.3 AG_Prop.3 \
	AG_Timeout.3 AG_View.3 AG_Config.3

CFLAGS+=${SDL_CFLAGS} ${SMPEG_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS} \
	${PTHREADS_XOPEN_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
