TOP=..
include ${TOP}/Makefile.config

LIB=		ag_core
LIB_MAJOR=	4
LIB_MINOR=	0
LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"9d6bb497-1d2a-4ac0-9f7b-7be68b9077e8"
LIB_LINKS=	pthreads winmm

SRCS=	variable.c config.c core.c error.c event.c object.c prop.c rcs.c \
	timeout.c class.c cpuinfo.c load_den.c \
	data_source.c load_string.c load_version.c \
	snprintf.c vsnprintf.c vasprintf.c \
	net_client.c net_command.c net_fgetln.c net_server.c \
	dir.c md5.c sha1.c rmd160.c file.c string_compat.c dso.c tree.c \
	time.c time_dummy.c time_gettimeofday.c time_win32.c time_condwait.c \
	db.c dbobject.c tbl.c getopt.c exec.c

MAN3=	AG_Intro.3 AG_Core.3 AG_Event.3 AG_Object.3 AG_Prop.3 AG_Timeout.3 \
	AG_Config.3 AG_Version.3 AG_DataSource.3 AG_Error.3 AG_Threads.3 \
	AG_CPUInfo.3 AG_ByteSwap.3 AG_Queue.3 AG_Limits.3 AG_DSO.3 AG_File.3 \
	AG_List.3 AG_Variable.3 AG_Time.3 AG_Tbl.3 AG_Getopt.3 AG_Execute.3
MAN5=	AG_DEN.5

CFLAGS+=-D_AGAR_CORE_INTERNAL ${PTHREADS_XOPEN_CFLAGS} \
	${ALTIVEC_CHECK_CFLAGS} ${GETTEXT_CFLAGS} ${DSO_CFLAGS} \
	${DB4_CFLAGS} ${CLOCK_CFLAGS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
