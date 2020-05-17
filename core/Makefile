TOP=..

include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc

LIB=		ag_core
LIB_CURRENT=	6
LIB_REVISION=	0
LIB_AGE=	0
LIB_INSTALL=	Yes
LIB_GUID=	"9d6bb497-1d2a-4ac0-9f7b-7be68b9077e8"
LIB_LINKS=	${CORE_LIBS}
LIBS=		${CORE_LIBS}

MAN3=	AG_ByteSwap.3 AG_CPUInfo.3 AG_Config.3 AG_Core.3 AG_DSO.3 \
	AG_DataSource.3 AG_Db.3 AG_Error.3 AG_Event.3 AG_EventLoop.3 \
	AG_Execute.3 AG_File.3 AG_Getopt.3 AG_Intro.3 AG_Limits.3 \
	AG_Object.3 AG_Queue.3 AG_String.3 AG_Tbl.3 AG_TextElement.3 \
	AG_Threads.3 AG_Time.3 AG_Timer.3 AG_User.3 AG_Variable.3 \
	AG_Version.3

SRCS=	byteswap.c class.c config.c core.c cpuinfo.c data_source.c \
	db.c dir.c dso.c error.c event.c exec.c file.c getopt.c \
	load_integral.c load_real.c load_string.c load_version.c \
	object.c string.c tbl.c text.c time.c time_dummy.c timeout.c \
	threads.c vasprintf.c vsnprintf.c user.c user_dummy.c \
	user_getenv.c variable.c vec.c \
	${SRCS_CORE}

CFLAGS+=${CORE_CFLAGS} -D_AGAR_CORE_INTERNAL

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
