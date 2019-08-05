TOP=..

include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/math/Makefile.inc
include ${TOP}/rg/Makefile.inc

LIB=		ag_map
LIB_CURRENT=	6
LIB_REVISION=	0
LIB_AGE=	0
LIB_INSTALL=	Yes
LIB_GUID=	"da8b3181-d485-4ef0-a68a-b747f08b5745"
LIB_LINKS=	${RG_LINKS_STATIC} ${AGMATH_LINKS_STATIC} ${GUI_LINKS_STATIC} \
		${CORE_LINKS_STATIC} ${MAP_LIBS}

MAN3=	MAP.3 MAP_Actor.3 MAP_View.3

SRCS=	fill.c flip.c invert.c map.c mapedit.c mapview.c nodemask.c \
	nodesel.c refsel.c tool.c insert.c ginsert.c eraser.c actor.c

CFLAGS+=${RG_CFLAGS} \
	${AGMATH_CFLAGS} \
	${GUI_CFLAGS} \
	${CORE_CFLAGS} \
	${MAP_CFLAGS} -D_AGAR_MAP_INTERNAL

#LIBS=	-L../rg   -lag_rg   ${RG_LIBS} \
#	-L../math -lag_math ${AGMATH_LIBS} \
#	-L../gui  -lag_gui  ${GUI_LIBS} \
#	-L../core -lag_core ${CORE_LIBS} \
#	${MAP_LIBS}

include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
