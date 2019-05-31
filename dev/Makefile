TOP=..

include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

LIB=		ag_dev
LIB_CURRENT=	6
LIB_REVISION=	0
LIB_AGE=	0
LIB_INSTALL=	Yes
LIB_GUID=	"e0baea2d-5cb8-4a7a-9120-8352f36109e7"
LIB_LINKS=	${CORE_LINKS_STATIC} ${GUI_LINKS_STATIC} ${DEV_LIBS}

SRCS=	dev.c browser.c timeouts.c uniconv.c view_params.c cpuinfo.c \
	config.c object.c classes.c

CFLAGS+=${GUI_CFLAGS} \
	${CORE_CFLAGS} \
	${DEV_CFLAGS} -D_AGAR_DEV_INTERNAL

LIBS=	-L../gui -lag_gui ${GUI_LIBS} \
	-L../core -lag_core

include ${TOP}/mk/build.lib.mk
