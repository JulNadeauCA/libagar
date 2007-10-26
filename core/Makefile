TOP=..
include ${TOP}/Makefile.config

LIB=		ag_core
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"9d6bb497-1d2a-4ac0-9f7b-7be68b9077e8"
LIB_LINKS=	SDL

SRCS=	config.c core.c error.c event.c object.c prop.c rcs.c \
	timeout.c typesw.c cpuinfo.c load_color.c load_den.c \
	data_source.c load_string.c load_surface.c load_version.c \
	load_xcf.c \
	setenv.c strlcat.c strlcpy.c strsep.c asprintf.c vasprintf.c \
	gethostname.c snprintf.c arc4random.c dir.c md5.c sha1.c rmd160.c \
	file.c

MAN3=	AG_Intro.3 AG_Core.3 AG_Event.3 AG_Object.3 AG_Prop.3 AG_Timeout.3 \
	AG_Config.3 AG_Version.3
MAN5=	AG_DEN.5

CFLAGS+=-I.. ${SDL_CFLAGS} ${X11_CFLAGS} ${JPEG_CFLAGS} \
	${PTHREADS_XOPEN_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
