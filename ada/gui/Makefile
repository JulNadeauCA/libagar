TOP=	../..
include Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

LIB=		ag_ada_gui
LIB_CURRENT=	6
LIB_REVISION=	0
LIB_AGE=	0
LIB_SHARED=	Yes
LIB_INSTALL=	Yes
LIB_GUID=	"7f5b4d4e-9561-11e8-965d-ac9e17b8d45b"
LIB_LINKS=	${CORE_LINKS_STATIC}
CFLAGS+=	${AGAR_CFLAGS} ${AGAR_ADA_CORE_CFLAGS} ${AGAR_CORE_CFLAGS}
LIBS+=		${AGAR_LIBS} ${AGAR_ADA_CORE_LIBS} ${AGAR_CORE_LIBS}

SRCS=		agar-init_gui.adb \
		agar-input_device.ads \
		agar-keyboard.ads \
		agar-mouse.adb \
		agar-surface.adb \
		agar-widget.adb \
		agar-text.adb

CONFIGSCRIPTS=	agar-ada-config
PCMODULES=	agar-ada.pc
ADAPREPFILE=	agar.def

DATAFILES=	LICENSE \
		${ADAPREPFILE}

CLEANDIRFILES+=	${ADAPREPFILE}

include ${TOP}/mk/build.lib.mk
