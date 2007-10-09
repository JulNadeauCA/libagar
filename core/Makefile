TOP=..
include ${TOP}/Makefile.config

LIB=		ag_core
LIB_SHARED=	Yes
LIB_STATIC=	No
SHARESRC=	core-icons.den

SRCS=	config.c core.c error.c event.c math.c iconmgr.c \
	object.c prop.c rcs.c timeout.c typesw.c view.c cpuinfo.c \
	load_color.c load_den.c load_integral.c load_netbuf.c \
	load_real.c load_string.c load_surface.c load_version.c \
	load_xcf.c

MAN3=	AG_Intro.3 AG_Core.3 AG_Event.3 AG_Object.3 AG_Prop.3 \
	AG_Timeout.3 AG_View.3 AG_Config.3 AG_Version.3
MAN5=	AG_XCF.5 AG_DEN.5

CFLAGS+=${SDL_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS} \
	${PTHREADS_XOPEN_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
include ${TOP}/mk/build.proj.mk
