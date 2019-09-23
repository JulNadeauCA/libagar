TOP=..

include ${TOP}/Makefile.config
include ${TOP}/core/Makefile.inc
include ${TOP}/gui/Makefile.inc

LIB=		ag_micro_gui
LIB_CURRENT=	6
LIB_REVISION=	0
LIB_AGE=	0
LIB_INSTALL=	Yes
LIB_GUID=	"a4f083ca-d2d5-11e9-b11c-ac9e17b8d45b"
LIB_LINKS=	${CORE_LINKS_STATIC} ${MICROGUI_LIBS}

#MAN3=

#SRCS=	drv_bbc.c drv_c64.c drv_c128.c drv_geos.c drv_nes.c
SRCS=	gui.c drv.c drv_dummy.c drv_c64.c \
	colors.c geometry.c primitive.c \
	widget.c window.c

#	box.c button.c checkbox.c combo.c console.c file_dlg.c
#	fixed.c fixed_plotter.c graph.c label.c notebook.c numerical.c
#	pane.c pixmap.c radio.c textbox.c scrollbar.c separator.c table.c
#	tlist.c toolbar.c

CFLAGS+=${CORE_CFLAGS} \
	${MICROGUI_CFLAGS} -D_AGAR_MICRO_INTERNAL

#LIBS=	-L../core -lag_core \
#	${GUI_LIBS}

#include .manlinks.mk
include ${TOP}/mk/build.lib.mk
#include ${TOP}/mk/build.man.mk
