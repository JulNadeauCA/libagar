TOP=..
include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc

LIB=		ag_core
LIB_CURRENT=	5
LIB_REVISION=	0
LIB_AGE=	0

LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"9d6bb497-1d2a-4ac0-9f7b-7be68b9077e8"
LIB_LINKS=	${CORE_LIBS}
LIBS=		${CORE_LIBS}
CFLAGS+=	-D_AGAR_CORE_INTERNAL ${CORE_CFLAGS}

SRCS=	${SRCS_CORE} variable.c config.c core.c error.c event.c object.c \
	prop.c timeout.c class.c cpuinfo.c data_source.c \
	load_string.c load_version.c vsnprintf.c vasprintf.c asprintf.c \
	dir.c md5.c sha1.c rmd160.c file.c string.c dso.c tree.c \
	time.c time_dummy.c db.c tbl.c getopt.c exec.c text.c user.c \
	user_dummy.c

MAN3=	AG_Intro.3 AG_Core.3 AG_Event.3 AG_Object.3 AG_Timer.3 \
	AG_Config.3 AG_Version.3 AG_DataSource.3 AG_Error.3 AG_Threads.3 \
	AG_CPUInfo.3 AG_ByteSwap.3 AG_Queue.3 AG_Limits.3 AG_DSO.3 AG_File.3 \
	AG_List.3 AG_Variable.3 AG_Time.3 AG_Tbl.3 AG_Getopt.3 AG_Execute.3 \
	AG_String.3 AG_User.3 AG_TextElement.3 AG_Net.3 AG_EventLoop.3

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
