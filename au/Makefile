TOP=	..

include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/au/Makefile.inc

LIB=		ag_au
LIB_CURRENT=	6
LIB_REVISION=	0
LIB_AGE=	0
LIB_INSTALL=	Yes
LIB_GUID=	"26f09743-b62e-4c74-abaf-0a4339a2b5d9"
LIB_LINKS=	${CORE_LINKS_STATIC} ${GUI_LINKS_STATIC} ${AU_LIBS}

MAN3=	AU.3 AU_DevOut.3 AU_Wave.3

SRCS=	${SRCS_AU} au.c au_wave.c au_dev_out.c

CFLAGS+=${GUI_CFLAGS} \
	${CORE_CFLAGS} \
	${AU_CFLAGS} -D_AGAR_AU_INTERNAL

LIBS=	${ENABLE_GUI_LIBS} \
	-L../core -lag_core ${CORE_LIBS} \
	${AU_LIBS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
