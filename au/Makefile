TOP=	..
include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/math/Makefile.inc
include ${TOP}/au/Makefile.inc

LIB=		ag_au
LIB_CURRENT=	5
LIB_REVISION=	0
LIB_AGE=	0

LIB_INSTALL=	Yes
LIB_SHARED=	Yes
LIB_GUID=	"26f09743-b62e-4c74-abaf-0a4339a2b5d9"
LIB_LINKS=	${CORE_LINKS_STATIC} ${GUI_LINKS_STATIC} \
		${AGMATH_LINKS_STATIC} ${AU_LIBS}

LIBS=		-L../gui -lag_gui

CFLAGS+=	-D_AGAR_AU_INTERNAL \
		${CORE_CFLAGS} ${GUI_CFLAGS} \
		${AGMATH_CFLAGS} ${AU_CFLAGS}

SRCS=	${SRCS_AU} au.c au_wave.c au_dev_out.c
MAN3=	AU.3 AU_DevOut.3 AU_Wave.3

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
