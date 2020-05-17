TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc

LIB=		ag_ada_core
LIB_CURRENT=	6
LIB_REVISION=	0
LIB_AGE=	0
LIB_SHARED=	Yes
LIB_INSTALL=	Yes
LIB_GUID=	"49d2fdbb-713c-11e8-b63b-ac9e17b8d45b"
LIB_LINKS=	${CORE_LINKS_STATIC}
CFLAGS+=	${AGAR_CORE_CFLAGS}
LIBS+=		${AGAR_CORE_LIBS}

SRCS=		ag_dso.c \
		ag_event.c \
		agar.ads \
		agar-config.adb \
		agar-data_source.adb \
		agar-dso.adb \
		agar-error.adb \
		agar-event.adb \
		agar-init.adb \
		agar-object.adb \
		agar-timer.adb \
		agar-types.ads

CONFIGSCRIPTS=	agar-ada-core-config
PCMODULES=	agar-ada-core.pc
ADAPREPFILE=	agar-core.def

DATAFILES=	LICENSE \
		${ADAPREPFILE}

CLEANFILES+=	Cow.obj
CLEANDIRFILES+=	${ADAPREPFILE}

include ${TOP}/mk/build.lib.mk
