TOP=..

include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc
include ${TOP}/math/Makefile.inc
include ${TOP}/sk/Makefile.inc

LIB=		ag_sk
LIB_CURRENT=	6
LIB_REVISION=	0
LIB_AGE=	0
LIB_INSTALL=	Yes
LIB_GUID=	"77c8a092-a7ef-4de0-a819-22d28cb8b608"
LIB_LINKS=	${AGMATH_LINKS_STATIC} ${GUI_LINKS_STATIC} ${CORE_LINKS_STATIC} \
		${SK_LIBS}

MAN3=	SK.3 SK_View.3

SRCS=	sk.c sk_view.c sk_tool.c sk_edit.c sk_select.c sk_solve.c sk_group.c \
	sk_intersect.c sk_dummy.c sk_point.c sk_line.c sk_circle.c sk_arc.c \
	sk_placement.c sk_annot.c sk_dimension.c sk_measure.c sk_geometry.c \
	sk_pixmap.c sk_polygon.c sk_gui.c

CFLAGS+=${AGMATH_CFLAGS} \
	${GUI_CFLAGS} \
	${CORE_CFLAGS} \
	${SK_CFLAGS} -D_AGAR_SK_INTERNAL

LIBS=	-L../math -lag_math ${AGMATH_LIBS} \
	-L../gui  -lag_gui  ${GUI_LIBS} \
	-L../core -lag_core ${CORE_LIBS} \
	${SK_LIBS}

include .manlinks.mk
include ${TOP}/mk/build.lib.mk
include ${TOP}/mk/build.man.mk
