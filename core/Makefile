TOP=..
include ${TOP}/Makefile.config

LIB=		ag_core
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"9d6bb497-1d2a-4ac0-9f7b-7be68b9077e8"
LIB_LINKS=	pthreads SDL

SRCS=	config.c core.c error.c event.c object.c prop.c rcs.c \
	timeout.c typesw.c cpuinfo.c load_color.c load_den.c \
	data_source.c load_string.c load_version.c \
	setenv.c snprintf.c vsnprintf.c vasprintf.c \
	gethostname.c arc4random.c dir.c md5.c sha1.c rmd160.c \
	file.c string_compat.c dso.c

MAN3=	AG_Intro.3 AG_Core.3 AG_Event.3 AG_Object.3 AG_Prop.3 AG_Timeout.3 \
	AG_Config.3 AG_Version.3 AG_DataSource.3 AG_Error.3 AG_Threads.3 \
	AG_CPUInfo.3 AG_ByteSwap.3 AG_Queue.3 AG_Limits.3
MAN5=	AG_DEN.5

CFLAGS+=-I.. ${SDL_CFLAGS} ${PTHREADS_XOPEN_CFLAGS} ${ALTIVEC_CHECK_CFLAGS} \
	${GETTEXT_CFLAGS} ${DSO_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
